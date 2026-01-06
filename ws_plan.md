# Polysonix v1.5: Wave Sequencing Implementation Plan

## Objective
Implement a per-voice, bytecode-driven Wave Sequencer with 8-byte steps, microtonal precision, and 16-bit logic flags. The implementation must be zero-allocation at runtime and fit within a 64KB static ROM budget.

## Philosophy
"Per-Cycle" logic for phase-perfect transitions.

## Phase 1: Data Structures & Constants (polysonix.h)

**Goal:** Define the storage format and logic flags.

**Actionables:**
- [ ] **Modify `polysonix.h` (Public Enums and Structs section)**:
    - [ ] Add 16-Bit Logic Flags macros.
    - [ ] Define `PxWaveSeqStep` struct (strictly 8-byte aligned).
    - [ ] Define `PxWaveSequence` struct.
    - [ ] Define `PX_NUM_WSEQ_BANKS` (128) and `PX_MAX_WSEQ_STEPS` (64).

**Detail:**
```c
// --- v1.5 Wave Sequencing Definitions ---

// 16-Bit Logic Flags
// --- FLOW CONTROL (Bits 0-3) ---
#define PX_WSEQ_END             (1 << 0)  // Stop sequence (hold step)
#define PX_WSEQ_LOOP            (1 << 1)  // Jump to Step 0
#define PX_WSEQ_PINGPONG        (1 << 2)  // Reverse direction at ends
#define PX_WSEQ_JUMP_RANDOM     (1 << 3)  // Jump to random step

// --- MODULATION / RESET (Bits 4-7) ---
#define PX_WSEQ_RESET_LFO       (1 << 4)  // Reset all LFO phases to 0
#define PX_WSEQ_RETRIG_ADSR     (1 << 5)  // Retrigger ADSR Attack
#define PX_WSEQ_GLIDE           (1 << 6)  // (Reserved for Glide logic - Future)
#define PX_WSEQ_LOCK_PHASE      (1 << 7)  // Hard Sync (Phase = 0)

// --- GENERATIVE (Bits 8-11) ---
#define PX_WSEQ_PROB_50_MUTE    (1 << 8)  // 50% chance to output silence
#define PX_WSEQ_PROB_50_SKIP    (1 << 9)  // 50% chance to skip step (0 time)
#define PX_WSEQ_RND_OCTAVE      (1 << 10) // Random +/- 1 Octave
#define PX_WSEQ_RND_WAVE        (1 << 11) // Random Wave Index

// --- GLITCH / TIMBRE (Bits 12-15) ---
#define PX_WSEQ_REVERSE_PLAY    (1 << 12) // Negative Frequency
#define PX_WSEQ_BITCRUSH        (1 << 13) // 2-bit quantization
#define PX_WSEQ_XMOD_SELF       (1 << 14) // Feedback FM
#define PX_WSEQ_RING_MOD        (1 << 15) // Square Wave Ring Mod (Octave Up)

// 8-Byte Step Structure (Aligned)
typedef struct {
    uint16_t wave_idx;        // 0-65535 (Waveform index)
    uint16_t duration_cycles; // 0-65535 (Number of oscillator cycles to hold this step)
    int16_t  pitch_offset;    // Cents: -32768 to +32767 (Relative to note pitch)
    uint16_t flags;           // Bitfield (PX_WSEQ_*)
} PxWaveSeqStep;

#define PX_MAX_WSEQ_STEPS 64
#define PX_NUM_WSEQ_BANKS 128

typedef struct {
    PxWaveSeqStep steps[PX_MAX_WSEQ_STEPS];
} PxWaveSequence;
```

**Memory Storage:**
*   In `polysonix.h` (Implementation section):
    ```c
    // Global ROM (Static Const to reside in Flash/RO)
    static const PxWaveSequence ROM_WAVE_SEQUENCES[PX_NUM_WSEQ_BANKS];
    ```

## Phase 2: Core Engine State (polysonix.h)

**Goal:** Update `Voice` and `PxPatch` structs to track sequence state.

**Actionables:**
- [ ] **Modify `PxPatch` struct:**
    - [ ] Add `int selected_sequence_id;` (Default: -1, Range: -1 to 127).
- [ ] **Modify `Voice` struct (Internal Data Structures):**
    - [ ] Add sequencer state fields.
    - [ ] Add per-step optimization cache fields.

**Detail (Voice Struct Additions):**
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
    float    step_pitch_ratio;  // Pre-calculated frequency multiplier (from cents)
    bool     step_mute_state;   // Latch for Mute probability

    // ... existing fields ...
} Voice;
```

## Phase 3: The Audio Loop (The Meat)

**Goal:** Inject logic into `PX_Process` and `PX_NoteOn_internal`.

**Actionables:**
- [ ] **Update `PX_NoteOn_internal`:**
    - [ ] Initialize `v->seq_id = s->patch.selected_sequence_id`.
    - [ ] If `seq_id >= 0`:
        - [ ] Reset `seq_step_idx = 0`, `seq_direction = 1`, `seq_cycles_counter = 0`, `seq_finished = false`.
        - [ ] **Load Step 0:** Call a helper to load step parameters (pitch ratio, flags, mute state, wave index).
        - [ ] **Safety Valve:** Clamp `duration_cycles` to a minimum of 1.
        - [ ] Handle initialization flags (e.g., `PX_WSEQ_LOCK_PHASE`).

- [ ] **Update `PX_Process` (Inside Voice Loop):**

    - [ ] **Logic Injection Point 1: Frequency Calc (Before Oscillator Update)**
        - [ ] Apply `v->frequency *= v->step_pitch_ratio`.
        - [ ] Ensure this happens *after* slide/tuning logic but *before* VM execution.
        - [ ] **Optimization Note:** `step_pitch_ratio` is calculated using `powf(2.0f, cents/1200.0f)` only on step changes. This is acceptable performance-wise; use fast approximation only if profiling shows issues.

    - [ ] **Logic Injection Point 2: Audio FX (After Bytecode/Interp, Before Filter/Pan)**
        - [ ] Apply Mute: `if (v->step_mute_state) raw_sample = 0.0f;`
        - [ ] Apply Bitcrush (if flag set): `raw_sample = floorf(raw_sample * 4.0f) * 0.25f;`
        - [ ] Apply Ring Mod (if flag set):
            ```c
            // Optimized "Ham Crazy" Square Ring Mod (Octave Up)
            float ring_mod = ((int)(v->phase * 4.0f) & 1) ? -1.0f : 1.0f;
            raw_sample *= ring_mod;
            ```
        - [ ] Apply Feedback FM (XMOD) (if flag set):
            ```c
            v->phase += raw_sample * 0.25f;
            if (v->phase >= 1.0f) v->phase -= 1.0f;
            else if (v->phase < 0.0f) v->phase += 1.0f;
            ```

    - [ ] **Logic Injection Point 3: Phase & Step Advancement (End of Loop)**
        - [ ] Detect cycle completion: `if (v->phase < previous_phase)` (taking `PX_WSEQ_REVERSE_PLAY` into account).
        - [ ] Increment `seq_cycles_counter`.
        - [ ] Check against duration: `if (seq_cycles_counter >= target_duration)`.
        - [ ] **Advance Step Logic:**
            - [ ] Update `seq_step_idx += seq_direction`.
            - [ ] **Handle Flags:**
                - [ ] `PX_WSEQ_LOOP`: `seq_step_idx = 0`.
                - [ ] `PX_WSEQ_PINGPONG`: Flip `seq_direction`.
                - [ ] `PX_WSEQ_END`: Set `seq_finished = true`.
                - [ ] `PX_WSEQ_JUMP_RANDOM`: `seq_step_idx = rand() % PX_MAX_WSEQ_STEPS` (or length).
            - [ ] **Load Next Step:**
                - [ ] Read `wave_idx`, `pitch_offset`, `flags` from ROM.
                - [ ] **Safety Valve:** Enforce `duration_cycles >= 1` to prevent infinite loops.
                - [ ] Update `v->source_wave_index`.
                - [ ] Update `v->step_pitch_ratio = powf(2.0f, pitch_offset / 1200.0f)`.
                - [ ] Update `v->step_flags`.
                - [ ] Calculate `v->step_mute_state` based on `PX_WSEQ_PROB_50_MUTE`.
            - [ ] Reset `seq_cycles_counter = 0`.
            - [ ] Handle Trigger Flags: `PX_WSEQ_RESET_LFO`, `PX_WSEQ_RETRIG_ADSR`.

## Phase 4: API & Control (polysonix.h)

**Goal:** Allow user control.

**Actionables:**
- [ ] **Update `PxCommandType`:** Add `PX_CMD_SET_SEQUENCE_ID`.
- [ ] **Add API Functions:**
    - [ ] `PX_API void PX_SetSequenceID(PxSynth* s, int seq_id);`
    - [ ] `PX_API int PX_GetSequenceID(PxSynth* s);`
- [ ] **Implement Command Handling:** Update `s->patch.selected_sequence_id` (clamp -1 to 127).
- [ ] **Update UI Snapshot:** Copy `selected_sequence_id` to `UISnapshot`.

## Phase 5: Content (ROM)

**Goal:** Populate `ROM_WAVE_SEQUENCES` with at least 4 diverse presets.

**Presets:**
- [ ] **Seq 0 (Basic):** 4 steps, Sine/Tri/Saw/Square, no pitch shift.
- [ ] **Seq 1 (Arp):** Major triad arpeggio (0, +400, +700 cents), 10 cycles each.
- [ ] **Seq 2 (Rhythmic):** Uses `PX_WSEQ_PROB_50_MUTE` for rhythmic gaps.
- [ ] **Seq 3 (FX):** Fast steps (2 cycles), `PX_WSEQ_BITCRUSH` and `PX_WSEQ_RING_MOD`.

## Phase 6: Verification Strategy

**Goal:** Ensure 100% integral outcome without a formal test suite.

**Actionables:**
- [ ] **Compile Check:** Verify `polysonix.h` compiles with `POLYSONIX_IMPLEMENTATION`.
- [ ] **Manual Test Harness:** Create a temporary `test_seq.c` that:
    - [ ] Creates a `PxSynth`.
    - [ ] Sets a sequence ID (e.g., Seq 1 Arp).
    - [ ] Calls `PX_NoteOn`.
    - [ ] Calls `PX_Process` for several blocks.
    - [ ] Prints `v->frequency` and `v->seq_step_idx` every 100 samples to verify progression.
- [ ] **Success Criteria:**
    - [ ] Step index advances.
    - [ ] Frequency changes according to pitch offset.
    - [ ] No crashes on step transitions.
