# Equal Tempered Wave Sequence Playback (Time-Locked Mode) Plan

This plan outlines the implementation of a "Time-Locked" Wave Sequencing mode in Polysonix. This feature decouples sequence step durations from oscillator cycles, allowing for deterministic, tempo-independent sequencing.

## 1. Overview

The current wave sequencing system is "cycle-locked," meaning step durations are defined in oscillator cycles. This causes sequences to play faster at higher pitches. The new "Time-Locked" mode will allow users to define a "Reference Frequency" (e.g., 440 Hz). When enabled, the engine recalculates the required number of cycles for each step based on the voice's current frequency to match the duration required at the reference frequency.

## 2. Analysis & Tradeoffs

### Pros
*   **Fixed, Predictable Tempo**: Sequence playback speed is consistent across all notes.
*   **Minimal Overhead**: Reuses the existing `cycles_counter` logic, avoiding expensive per-sample time accumulation.
*   **Backward Compatibility**: The feature is toggleable (off by default).

### Cons / Tradeoffs
*   **Precision**: Relying on integer cycle counters with a scaled target might introduce slight jitter compared to pure time accumulation, but is sufficient for musical timing.
*   **Pitch Slides**: Rapid pitch changes (unilegato) will dynamically alter the cycle target, keeping the *time* constant, which is the desired behavior.

## 3. Data Structure Updates

### `polysonix.h`

*   **`PxPatch` Struct**: Add global configuration for the time-locked mode.
    ```c
    bool  wseq_fixed_time;      // Enable Time-Locked mode
    float wseq_ref_freq;        // Reference frequency (default 440.0 Hz)
    ```

    *No new fields are added to `PxSeqState` (No bloat).*

### `PxCommandType`

*   Add new commands to `PxCommandType` enum:
    *   `PX_CMD_SET_WSEQ_FIXED_TIME`
    *   `PX_CMD_SET_WSEQ_REF_FREQ`

## 4. API Updates

*   **Public API Functions**:
    *   `PX_SetWSeqFixedTime(PxSynth* s, bool enabled)`
    *   `PX_GetWSeqFixedTime(PxSynth* s)`
    *   `PX_SetWSeqRefFreq(PxSynth* s, float freq)`
    *   `PX_GetWSeqRefFreq(PxSynth* s)`

## 5. Logic Implementation

### Audio Processing (`PX_Process`)
*   **Sequence Advancement Logic**:
    *   Inside the oscillator loop, where `sq->cycles_counter` is checked against `duration_cycles`:
    *   Calculate a `target_cycles` threshold:
        *   **If `wseq_fixed_time` is ON**:
            `target_cycles = step.duration_cycles * (v->frequency / wseq_ref_freq)`
        *   **If OFF**:
            `target_cycles = step.duration_cycles`
    *   Compare `sq->cycles_counter >= target_cycles`.
    *   This scales the requirement: Higher pitch = faster cycles = higher target = constant time.
*   **Interpolation**:
    *   The calculation for `t` (used for Amp Mod/Glide) uses the scaled `target_cycles` as the denominator.

## 6. Serialization

*   **`PX_UpdateUISnapshot`**:
    *   Copy `wseq_fixed_time` and `wseq_ref_freq` from `PxPatch` to the UI snapshot.
*   **`px_patching.h`**:
    *   Update `px_serialize_patch_impl` and `px_deserialize_patch_impl` to include `wseq_fixed_time` and `wseq_ref_freq`.

## 7. Pre-Commit Steps

*   Complete pre-commit steps to ensure proper testing, verification, review, and reflection are done.

## 8. Submit

*   Submit the changes with a descriptive commit message.
