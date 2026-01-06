# Polysonix v1.5: Wave Sequencing Integration Plan

## Objective
Implement a per-voice, bytecode-driven Wave Sequencer with 8-byte steps, microtonal precision, and 16-bit logic flags.

## Philosophy
"Per-Cycle" logic for phase-perfect transitions.

## Constraint
Zero memory allocation during runtime. 64KB static ROM.

---

## Phase 1: Data Structures & Constants

**Goal:** Define the storage format and logic flags without breaking existing code.

**Actionables:**
1.  **Modify `polysonix.h`**:
    *   Locate the "Public Enums and Structs" section (around line 50).
    *   Add the 16-Bit Logic Flags macros (`PX_WSEQ_END`, `PX_WSEQ_LOOP`, etc.).
    *   Add the `PxWaveSeqStep` struct definition (8-byte aligned).
    *   Add the `PxWaveSequence` struct definition.
    *   Declare the global ROM: `extern PxWaveSequence ROM_WAVE_SEQUENCES[PX_NUM_WSEQ_BANKS];`.

**Code Snippet:**
```c
// --- v1.5 Wave Sequencing Definitions ---

// 16-Bit Logic Flags
#define PX_WSEQ_END             (1 << 0)  // Stop sequence (hold step)
#define PX_WSEQ_LOOP            (1 << 1)  // Jump to Step 0
#define PX_WSEQ_PINGPONG        (1 << 2)  // Reverse direction at ends
#define PX_WSEQ_JUMP_RANDOM     (1 << 3)  // Jump to random step
#define PX_WSEQ_RESET_LFO       (1 << 4)  // Reset all LFO phases to 0
#define PX_WSEQ_RETRIG_ADSR     (1 << 5)  // Retrigger ADSR Attack
#define PX_WSEQ_GLIDE           (1 << 6)  // (Reserved for Glide logic)
#define PX_WSEQ_LOCK_PHASE      (1 << 7)  // Hard Sync (Phase = 0)
#define PX_WSEQ_PROB_50_MUTE    (1 << 8)  // 50% chance to output silence
#define PX_WSEQ_PROB_50_SKIP    (1 << 9)  // 50% chance to skip step (0 time)
#define PX_WSEQ_RND_OCTAVE      (1 << 10) // Random +/- 1 Octave
#define PX_WSEQ_RND_WAVE        (1 << 11) // Random Wave Index
#define PX_WSEQ_REVERSE_PLAY    (1 << 12) // Negative Frequency
#define PX_WSEQ_BITCRUSH        (1 << 13) // 2-bit quantization
#define PX_WSEQ_XMOD_SELF       (1 << 14) // Feedback FM
#define PX_WSEQ_RING_MOD        (1 << 15) // Square Wave Ring Mod (Octave Up)

// 8-Byte Step Structure (Aligned)
typedef struct {
    uint16_t wave_idx;        // 0-65535
    uint16_t duration_cycles; // 0-65535
    int16_t  pitch_offset;    // Cents: -32768 to +32767
    uint16_t flags;           // Bitfield
} PxWaveSeqStep;

#define PX_MAX_WSEQ_STEPS 64
#define PX_NUM_WSEQ_BANKS 128

typedef struct {
    PxWaveSeqStep steps[PX_MAX_WSEQ_STEPS];
} PxWaveSequence;

// Global ROM (Defined in implementation)
extern PxWaveSequence ROM_WAVE_SEQUENCES[PX_NUM_WSEQ_BANKS];
```

---

## Phase 2: Core Engine State

**Goal:** Update the Voice and Patch structs to track sequence state.

**Actionables:**
1.  **Modify `PxPatch` struct** in `polysonix.h`:
    *   Add `int selected_sequence_id;` (Default: -1).
    *   Add this field near other int fields (e.g., `filter_poles`).
2.  **Modify `Voice` struct** in `polysonix.h` (Internal Data Structures section):
    *   Add the sequencer state machine fields:
        *   `int seq_id;`
        *   `int seq_step_idx;`
        *   `int seq_direction;`
        *   `int seq_cycles_counter;`
        *   `bool seq_finished;`
    *   Add per-step cached values for optimization:
        *   `uint16_t step_flags;`
        *   `float step_pitch_ratio;`
        *   `bool step_mute_state;`
3.  **Update `PX_NoteOn_internal`**:
    *   Initialize the sequence state when a note starts.
    *   Copy `s->patch.selected_sequence_id` to `v->seq_id`.
    *   Set initial `seq_step_idx` to 0, `seq_direction` to 1, etc.
    *   **Crucial:** If `v->seq_id >= 0`, immediately load data for Step 0 (calculate pitch ratio, load flags, set wave index).

**Code Snippet (Voice Struct):**
```c
typedef struct Voice {
    // ... existing fields ...

    // --- v1.5 Sequencer State ---
    int  seq_id;                // -1 = Off
    int  seq_step_idx;          // Current step (0-63)
    int  seq_direction;         // 1 (Forward) or -1 (Backward)
    int  seq_cycles_counter;    // How many cycles played in this step
    bool seq_finished;          // True if END flag hit

    // --- Per-Step Cached Values (Optimization) ---
    uint16_t step_flags;        // Current flags
    float    step_pitch_ratio;  // Pre-calculated frequency multiplier
    bool     step_mute_state;   // Latch for Mute probability

    // ... existing fields ...
} Voice;
```

---

## Phase 3: The Audio Loop (The "Meat")

**Goal:** Inject the logic into `PX_Process` in `polysonix.h`.

**Actionables:**
1.  **Frequency Injection**:
    *   In `PX_Process`, find the "Step 5: Apply Modulations and Generate Audio" section.
    *   After `v->frequency` is calculated (based on tuning and LFOs), inject the sequencer pitch modification.
    *   `if (v->seq_id >= 0) v->frequency *= v->step_pitch_ratio;`
    *   Update `v->main_osc_vm_params.frequency` accordingly.
    *   **Optimization Note:** Calculating `powf(2.0f, cents/1200.0f)` for `step_pitch_ratio` only happens on step changes (not per sample), so standard `powf` is acceptable for v1.5. If CPU usage spikes on fast sequences, consider replacing `powf` with a fast approximation or lookup table.
2.  **Audio FX Injection**:
    *   Locate the `execute_bytecode` calls (both in the "Highest Quality" path and "Performance Modes" path).
    *   Immediately after `raw_sample` is calculated (or interpolated), inject the FX logic.
    *   Apply Mute, Bitcrush, Ring Mod, and Feedback FM (XMOD) based on `v->step_flags`.
3.  **Phase & Logic Injection**:
    *   At the end of the voice loop, where `v->phase` is updated.
    *   Modify phase update to support `PX_WSEQ_REVERSE_PLAY` (negative frequency).
    *   Detect cycle completion (phase wrapping).
    *   Inside the cycle completion block:
        *   Handle sequencer stepping logic (advance `seq_cycles_counter`).
        *   **Safety Valve:** When reading `duration_cycles` from ROM, enforce a minimum of 1 cycle to prevent infinite loops: `if (target_cycles < 1) target_cycles = 1;`.
        *   Check against `duration_cycles`.
        *   If step complete: Handle `PX_WSEQ_LOOP`, `PX_WSEQ_PINGPONG`, `PX_WSEQ_END`, `PX_WSEQ_JUMP_RANDOM`.
        *   Load next step data (wave index, pitch ratio, flags).
        *   Handle `PX_WSEQ_RESET_LFO`, `PX_WSEQ_RETRIG_ADSR`, `PX_WSEQ_LOCK_PHASE`.

**Code Snippet (Audio FX):**
```c
// v1.5 FX Processing
if (v->seq_id >= 0) {
    if (v->step_mute_state) raw_sample = 0.0f;

    if (v->step_flags & PX_WSEQ_BITCRUSH) {
        raw_sample = floorf(raw_sample * 4.0f) * 0.25f;
    }

    if (v->step_flags & PX_WSEQ_RING_MOD) {
        // Optimized "Ham Crazy" Square Ring Mod
        float ring_mod = ((int)(v->phase * 4.0f) & 1) ? -1.0f : 1.0f;
        raw_sample *= ring_mod;
    }

    if (v->step_flags & PX_WSEQ_XMOD_SELF) {
        v->phase += raw_sample * 0.25f; // Feedback FM
        // Wrap phase immediately
        if (v->phase >= 1.0f) v->phase -= 1.0f;
        else if (v->phase < 0.0f) v->phase += 1.0f;
    }
}
```

---

## Phase 4: API & Control

**Goal:** Allow the UI/User to select sequences.

**Actionables:**
1.  **Update `PxCommandType` enum** in `polysonix.h`:
    *   Add `PX_CMD_SET_SEQUENCE_ID`.
2.  **Add Public API functions**:
    *   `PX_API void PX_SetSequenceID(PxSynth* s, int seq_id);`
    *   `PX_API int PX_GetSequenceID(PxSynth* s);`
3.  **Implement Command Handling** in `PX_ProcessCommands`:
    *   Add case for `PX_CMD_SET_SEQUENCE_ID`.
    *   Update `s->patch.selected_sequence_id` (clamp between -1 and 127).
4.  **Update UI Snapshot**:
    *   Add `selected_sequence_id` to `PxPatch` copy in `UISnapshot` and update it in `PX_UpdateUISnapshot`.

---

## Phase 5: Content (ROM)

**Goal:** Populate the static memory so the feature actually makes sound.

**Actionables:**
1.  **Define Global ROM** in `polysonix.h` (Implementation section):
    *   Declare as `static const` to ensure it stays in Flash/RO memory and saves RAM.
    *   `static const PxWaveSequence ROM_WAVE_SEQUENCES[PX_NUM_WSEQ_BANKS] = { ... };`
2.  **Populate Starter Sequences**:
    *   **Seq 0 (Basic):** 4 steps, simple wave swap, no pitch change.
    *   **Seq 1 (Rhythmic):** 8 steps, using MUTE probability for gaps.
    *   **Seq 2 (Glitch):** Fast duration, BITCRUSH + RING_MOD flags.
    *   **Seq 3 (Generative):** JUMP_RANDOM + RND_OCTAVE flags.

---

## Success Criteria
*   **Legacy Safety:** Loading the synth with `seq_id = -1` sounds exactly like v1.4.
*   **Performance:** No audible CPU spike when a sequence advances a step.
*   **Stability:** `JUMP_RANDOM` or `PINGPONG` never causes an array out-of-bounds crash.
*   **Audio Quality:** Ring Mod and Bitcrush sound aggressive but clean (no aliasing popping from bad phase wraps).
