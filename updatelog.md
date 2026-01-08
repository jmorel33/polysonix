# Update Log

## v1.6.0 (2026-01-XX)
**Major Feature Update: Triple Oscillator Architecture**

*   **Architecture:**
    *   Transitioned from single-oscillator voices to a **Triple Oscillator** architecture (`PX_MAX_OSC_PER_VOICE = 3`).
    *   Each oscillator has independent controls for Waveform, Coarse Tuning (-24 to +24 semitones), Fine Tuning (-100 to +100 cents), Mix Level, Stereo Pan, and Enable state.
    *   Updated `PxPatch` and `Voice` structures to accommodate the new oscillator arrays.

*   **Wave Sequencing:**
    *   **Independent Sequencing:** Each of the 3 oscillators now has its own independent Wave Sequencer state (`PxSeqState`), allowing complex polyrhythmic and multi-timbral layering within a single voice.
    *   **Feature Preservation:** All v1.5 Wave Sequencer features (Glitch effects, Probability, Glide modes, Phase Locking) are preserved and function independently per oscillator.
    *   New API: `PX_SetOscSequence(synth, osc_idx, seq_id)` allows assigning different sequences to different oscillators.

*   **Audio Engine:**
    *   Refactored `PX_Process` to iterate through the 3 oscillators, summing their output into a stereo mix before the filter stage.
    *   **Unilegato Fix:** Separated base frequency calculation from modulation to prevent "double modulation" artifacts during slides. Global pitch modulation (LFO, ADSR) is now applied to the interpolated frequency, ensuring smooth and stable portamento.
    *   **Stereo Filtering:** Added `filter_instance_r` to the voice structure to support true stereo processing (e.g., for panned oscillators).

*   **API Updates:**
    *   Added `PX_SetOscEnabled`, `PX_SetOscWave`, `PX_SetOscMix`, `PX_SetOscPan`.
    *   Updated `PX_SetOscCoarseTune` and `PX_SetOscFineTune` to take an `osc_idx` parameter.
    *   Added `PX_GetOscSequence`.
    *   Legacy `PX_NoteOn` maps the `wave_idx` argument to Oscillator 0.
    *   Legacy `PX_SetSequenceID` maps to Oscillator 0.

*   **Diagnostics:**
    *   Updated `PxVoiceInfo` to include `active_wave_indices[3]`, enabling real-time monitoring of which waveform each oscillator is currently playing (useful for visualizing Wave Sequence progress).

## v1.5.0 (2026/01/06)

This major release introduces the **Wave Sequencer**, a powerful per-voice sequencing engine for rhythmic, glitch, and generative sound design.

### Features
*   **Wave Sequencer:**
    *   **Per-Voice Logic:** Each voice runs its own independent sequencer instance, allowing for polyrhythmic and phase-perfect step transitions.
    *   **Per-Cycle Precision:** Step advancement and logic are evaluated every waveform cycle, ensuring tight synchronization with the oscillator phase.
    *   **Global Settings:**
        *   **End Actions:** Loop, PingPong, Stop, Hold, or Reverse.
        *   **Glide Modes:** `STEP` (linear glide over step duration) and `SMOOTH` (continuous 1-pole portamento).
        *   **FX:** Bitcrush (1-8 bits), Ring Mod, and XMod (Feedback FM), with modulation depth control via standard sources (Velocity, Mod Wheel, Aftertouch).
        *   **Probabilities:** Global "scores" (0-100%) for randomly muting or skipping steps.
    *   **Per-Step Flags:**
        *   **Control:** `JUMP_RANDOM`, `END`, `LOOP_POINT`.
        *   **Generative:** `USE_PROB_MUTE`, `USE_PROB_SKIP`, `USE_RND_OCTAVE`, `USE_RND_WAVE`.
        *   **Modulation:** `RESET_LFO`, `RETRIG_ADSR` (to specific phase), `LOCK_PHASE` (Hard Sync).
        *   **FX:** `BITCRUSH`, `RING_MOD`, `XMOD`, `REVERSE_PLAY`.

*   **Audio Engine Improvements:**
    *   **Thread-Safe PRNG:** Replaced `rand()` in the audio path with a context-aware Linear Congruential Generator (`px_rand`), seeded deterministically per-voice.
    *   **Optimized DSP:** Bitcrush and pitch ratios are pre-calculated per step to minimize CPU load.

### API Changes
*   **New Functions:**
    *   `PX_SetSequenceID(s, seq_id)`: Selects the active sequence (-1 for off).
    *   `PX_GetSequenceID(s)`
*   **New Structs:** `PxWaveSequence` and `PxWaveSeqStep` defined in `polysonix.h`.
*   **ROM Storage:** Sequences are stored in `ROM_WAVE_SEQUENCES` (Flash/RO memory friendly).

### Backward Compatibility
*   **Default State:** Sequence ID defaults to -1 (Off). Existing patches behave identically to previous versions.

## v1.4.6 (2026/01/05)

This update completes the core analog voicing section by introducing **Per-Oscillator Coarse & Fine Tuning**.

### Features
*   **Per-Oscillator Tuning:**
    *   **Coarse Tuning:** Each waveform can now be detuned by ±24 semitones (±2 octaves) independently.
    *   **Fine Tuning:** Each waveform can be fine-tuned by ±100 cents (±1 semitone) for rich detuning, beating, and chorus effects.
    *   **Per-Voice Logic:** Tuning offsets are applied at the voice level *before* modulation, ensuring stable intervals even when pitch is modulated.
    *   **Applications:** Enables classic analog techniques such as:
        *   Sub-octave bass layering (set coarse to -12 or -24).
        *   Fifth intervals (set coarse to +7).
        *   Supersaw-style detuning (using fine tune).
        *   Fixed-frequency drones or clusters.

### API Changes
*   **New Functions:**
    *   `PX_SetOscCoarseTune(s, wave_idx, semitones)`: Sets coarse tuning (-24 to +24).
    *   `PX_GetOscCoarseTune(s, wave_idx)`
    *   `PX_SetOscFineTune(s, wave_idx, cents)`: Sets fine tuning (-100 to +100).
    *   `PX_GetOscFineTune(s, wave_idx)`
*   **Struct Update:** `PxPatch` now includes `osc_coarse_semitones` and `osc_fine_cents` arrays.

### Backward Compatibility
*   **Defaults:** All tuning offsets default to `0.0`, ensuring that existing patches play at standard pitch and sound identical to previous versions.

## v1.4.5 (2026/01/05)

This update refines the modulation system with **Non-Linear Response Curves** for velocity and aftertouch, plus a new **Keyboard Tracking** modulation source.

### Features
*   **Response Curves:**
    *   **Per-Source Customization:** Velocity and Aftertouch (Channel & Poly) can now be mapped using one of four curves: `Linear` (default), `Exponential`, `Logarithmic`, or `S-Curve`.
    *   **Global Application:** Curves are applied globally per patch, transforming the raw input (0.0–1.0) before it enters the modulation matrix.
    *   **Curve Types:**
        *   `PX_CURVE_LINEAR`: 1:1 mapping.
        *   `PX_CURVE_EXP`: Sensitive at high velocities/pressures (power of 2).
        *   `PX_CURVE_LOG`: Sensitive at low velocities/pressures (logarithmic).
        *   `PX_CURVE_S`: Smooth ease-in/ease-out response.
*   **Keyboard Tracking Source:**
    *   **New Source:** Added `PX_MOD_SRC_KEY_TRACK` to the modulation matrix.
    *   **Functionality:** Generates a modulation signal based on the note pitch, normalized relative to C4 (MIDI 60).
    *   **Range:** -1.0 (low keys) to +1.0 (high keys), allowing key position to modulate any parameter (e.g., filter cutoff, LFO speed).

### API Changes
*   **New Functions:**
    *   `PX_SetVelocityCurve(s, curve)` / `PX_GetVelocityCurve(s)`
    *   `PX_SetAftertouchCurve(s, curve)` / `PX_GetAftertouchCurve(s)`
*   **New Enums:** `PxCurveType` (`PX_CURVE_LINEAR`, `PX_CURVE_EXP`, etc.).
*   **Updated Enum:** Added `PX_MOD_SRC_KEY_TRACK` to `PxModSource`.

### Backward Compatibility
*   Defaults to `PX_CURVE_LINEAR` and 0.0 amount for Key Track modulation, preserving existing patch behavior.

## v1.4.4 (2026/01/05)

This update introduces a **Global Post-Filter**, allowing final tone shaping of the entire mix before the limiter.

### Features
*   **Global Post-Filter:**
    *   **Architecture:** Adds a new filter stage after voice mixing and before the master limiter.
    *   **Stereo Processing:** Uses two independent filter instances (Left/Right) to ensure correct stereo signal processing without state crosstalk.
    *   **Full Control:** Supports all existing filter modes (LP, HP, BP, Notch, Allpass, Combo) and slopes (6/12/18/24 dB/oct).
    *   **API:** New functions to control the global filter: `PX_SetGlobalFilterEnabled`, `PX_SetGlobalFilterParam`, `PX_SetGlobalFilterMode`.
*   **Optimization:**
    *   Calculates filter coefficients once per block and shares them between Left/Right channels for efficiency.

### Backward Compatibility
*   **Disabled by Default:** The global filter is disabled in default patches (`global_filter_enabled = false`), ensuring existing projects sound identical.
*   **Struct Update:** `PxSynth` and `PxPatch` structures have been updated to include global filter state.

## v1.4.3 (2026/01/05)

This update delivers **Full Combo Filter Support** at all filter slopes, including the gentle **6 dB/oct (1-pole)** setting.

### Features
*   **True Combo Modes at 6 dB/oct:**
    *   **New Architecture:** Implemented parallel independent 1-pole filter stages (`combo_lp_state`, `combo_hp_state`) specifically for combo modes (`LP+BP`, `LP+HP`, `BP+HP`) when `poles == 1`.
    *   **Accurate Summation:** Ensures mathematically correct and musically useful signal summation for these combinations, which was previously limited or approximated at 6 dB/oct.
    *   **Optimized:** Standard single modes (LP, HP, BP, Notch, Allpass) continue to use the efficient shared-state path.

### Backward Compatibility
*   Fully backward compatible. Existing patches using steeper slopes (12/18/24 dB/oct) or single modes at 6 dB/oct use existing code paths and sound identical.

## v1.4.2 (2026/01/05)

This update introduces full support for **1-Pole (6 dB/oct)** filtering, enabling gentler, broader tonal shaping.

### Features
*   **True 1-Pole Filter Support:**
    *   **New Pole Option:** `PX_SetFilterParam(s, PX_FILTER_PARAM_POLES, 1.0f)` now enables a true 6 dB/oct slope.
    *   **Unified Modes:** Works with existing `LP`, `HP`, `BP`, and `Allpass` modes. (Note: BP and Allpass are 1-pole approximations).
    *   **Optimized Path:** Internally bypasses the standard SVF stages when running in 1-pole mode for efficiency.

### Backward Compatibility
*   Fully backward compatible. Existing patches using 2, 3, or 4 poles are unaffected.
*   `PX_FILTER_PARAM_POLES` clamping has been updated to accept values down to `1.0`.

## v1.4.1 (2026/01/05)

This update adds comprehensive support for **Polyphonic Aftertouch** (per-note pressure) within the unified Modulation Matrix.

### Features
*   **Polyphonic Aftertouch:**
    *   **New Source:** Added `PX_MOD_SRC_POLY_AFTERTOUCH` to the modulation matrix.
    *   **Per-Note Control:** Allows modulating parameters (e.g., filter cutoff, timbre) independently for each held note based on its individual pressure.
    *   **Voice Handling:** Implemented per-voice pressure storage (`poly_aftertouch_pressure` in `Voice` struct). Pressure state is automatically reset to 0.0 when a voice is triggered or stolen to prevent state pollution.
*   **Documentation:**
    *   Updated `@section mod_matrix` in `polysonix.h` with new usage examples for Polyphonic Aftertouch and Pitch Bend.
    *   Clarified documentation for `PX_SetPitchBendRange`.

### API Changes
*   **New Function:** `PX_PolyAftertouch(PxSynth* s, int key_id, float pressure)`: Updates the pressure for the active voice corresponding to `key_id`.
*   **New Command:** `PX_CMD_POLY_AFTERTOUCH`: Internal command to safely handle pressure updates from the API thread.

### Backward Compatibility
*   Fully backward compatible. The new modulation source defaults to 0.0, and existing code not calling `PX_PolyAftertouch` will function unchanged.

## v1.4 (2026/01/05)

This release completes the unification of the modulation system by treating **Mod Wheel** and **Pitch Bend** as first-class citizens in the Modulation Matrix.

### Features
*   **Unified Modulation Matrix:**
    *   **New Sources:** Added `PX_MOD_SRC_MODWHEEL` (CC #1) and `PX_MOD_SRC_PITCHBEND` to the matrix.
    *   **Mod Wheel:** Maps MIDI CC #1 (0.0 to 1.0) to any destination (e.g., LFO Depth for vibrato, Filter Cutoff).
    *   **Pitch Bend:** Maps Pitch Bend (normalized 0.0-1.0 to bipolar -1.0 to +1.0) to any destination.
    *   **Pitch Bend Range:** Added `pitchbend_range_semitones` (default 2.0) to `PxPatch` for easier scaling when routing pitch bend to frequency.
*   **Safety & Polish:**
    *   **Input Validation:** `PX_SetModMatrixSlot` now clamps modulation amounts to [-1.0, 1.0] and logs errors to `stderr` for invalid slot/source/dest indices.
    *   **Documentation:** Added detailed usage examples for the Modulation Matrix in `polysonix.h`.

### API Changes
*   **New Control Functions:**
    *   `PX_ControlChange(s, cc_num, value)`: Handles MIDI CC messages (currently only CC #1 Mod Wheel is routed).
    *   `PX_PitchBend(s, value)`: Handles Pitch Bend messages (accepts 0.0-1.0 normalized).
    *   `PX_SetPitchBendRange(s, semitones)` / `PX_GetPitchBendRange(s)`: Helper for pitch bend scaling.
*   **Struct Updates:** Added `modwheel_value` and `pitchbend_value` to `PxSynth`, and `pitchbend_range_semitones` to `PxPatch`.

### Backward Compatibility
*   New modulation sources are zero by default.
*   The modulation matrix defaults to disabled slots, ensuring existing patches sound unchanged.

## v1.3 (2026/01/05)

This major update introduces a full **Modulation Matrix** for Velocity and Channel Aftertouch, replacing the previous hard-wired routings with a flexible, 16-slot routing system.

### Features
*   **Modulation Matrix:** 16-slot matrix allowing Velocity and Channel Aftertouch to be routed to freely selectable destinations.
    *   **Sources:** `PX_MOD_SRC_VELOCITY`, `PX_MOD_SRC_AFTERTOUCH`.
    *   **Destinations:**
        *   ADSR 1/2/3 parameters (Attack, Decay, Sustain, Release times/levels).
        *   LFO 1/2/3 Frequency and Depth.
        *   Oscillator Parameters (modA, modB, modC).
    *   **Exponential Scaling:** ADSR time modulations use natural-sounding exponential scaling.
    *   **Flexible Amounts:** Each slot has an independent amount (-1.0 to +1.0).

### API Changes
*   **New Matrix API:**
    *   `PX_SetModMatrixSlot(s, slot, src, dest, amount)`
    *   `PX_EnableModMatrixSlot(s, slot, enabled)`
    *   `PX_ClearModMatrix(s)`
*   **Removed Hard-wired Functions:** The previous Velocity/Aftertouch setters (e.g., `PX_SetVelocityToAmp`, `PX_SetAftertouchToFilterCutoff`) have been removed from the internal logic. The API functions remain for compilation compatibility but perform no action. Users should migrate to the Modulation Matrix.

### Backward Compatibility
*   **Default State:** The matrix defaults to all slots disabled (amount 0.0). Existing patches that did not use the specific v1.2 velocity features will sound identical.
*   **Migration:** Code using v1.2 velocity functions must be updated to use `PX_SetModMatrixSlot` to achieve similar results.

## v1.2 (2026/01/04)

This release introduces expressive capabilities with full support for **MIDI Velocity** and **Channel Aftertouch**.

### Features
*   **MIDI Velocity Support:** The engine now responds to note velocity (0.0 - 1.0).
    *   **Amplitude Scaling:** Velocity can scale the note's volume (`VelocityToAmp`).
    *   **Filter Brightness:** Harder hits can open the filter cutoff (`VelocityToFilterCutoff`).
    *   **Attack Time Scaling:** High velocity can shorten the ADSR attack time for punchier sounds (`VelocityAttackScaling`).
    *   **Timbre Modulation:** Direct mapping of velocity to `modA` (Param1) of the waveform bytecode (`VelocityToParam1`).
*   **Channel Aftertouch:** Added support for monophonic channel pressure (`PX_ChannelAftertouch`).
    *   **Filter Sweep:** Pressure can modulate filter cutoff (`AftertouchToFilterCutoff`).
    *   **Vibrato Depth:** Pressure can introduce pitch modulation (`AftertouchToVibrato`).

### API Changes
*   **Updated `PX_NoteOn`:** The signature has changed to accept a 5th argument: `float velocity`.
*   **New `PX_NoteOnLegacy`:** A helper function provided for backward compatibility with the old 4-argument signature (defaults to full velocity).
*   **New Control Functions:** Added setters and getters for all new velocity and aftertouch parameters (e.g., `PX_SetVelocityToAmp`, `PX_SetAftertouchToVibrato`).

### Backward Compatibility
*   All new modulation parameters default to `0.0`. Existing patches will sound exactly the same until these features are explicitly enabled.

## v1.1.8 (2026/01/04)

This release seals the current codebase as **Version 1.0Alpha1**, marking a significant milestone in stability and mathematical precision.

### Fixes
*   **ADSR Envelope Accuracy:** Corrected the decay and release calculations in `ADSR_Update`. Replaced the linear single-step multiplier application with an exponential `powf` calculation based on the actual sample count (`num_steps`). This fixes issues where envelopes played slower than intended during block-based updates.
*   **Interpolation Phase Continuity:** Improved the phase tracking logic in `PX_Process` for oscillator interpolation. The start phase for a new processing block is now preserved from the previous block's end phase (`phase_at_interp_end`), rather than being back-calculated from the current frequency. This ensures seamless audio continuity even under heavy frequency modulation.
*   **LFO Synchronization:** Fixed a bug in `PX_NoteOn_internal` where LFOs configured to not reset on key press (`reset_on_key_on = false`) failed to synchronize with the global LFO state. These LFOs now correctly inherit the phase and VM state from the master `template_lfo_instances`, ensuring true free-running behavior across voice reuse.

## v1.1.7 (2026/01/03)

### Fixes
*   **Waveform Generation Precision:** In `polysonix_wave`, fixed an issue where CPU rendering of waveforms was previously quantized to 16-bit integers. This has been updated to use 32-bit floating-point precision, eliminating quantization artifacts and significantly improving audio fidelity.

## v1.1.6 (2025/11/29)
*   **Performance optimization:** Implemented Direct Threaded Code (computed gotos) for VM dispatch.
*   **Performance optimization:** Added register caching for Instruction Pointer (IP) and Stack Pointer (SP).
*   **Performance optimization:** Added PX_LIKELY/PX_UNLIKELY branch prediction macros.
*   Verified ~24% performance improvement in benchmark suite.

## v1.1.5 (2025/07/14)
*   Fully implemented lock-free thread-safety via a command queue for control and a snapshot buffer for UI data.

## v1.1.4 (2025/07/12)
*   Enhanced filter engine with combo modes (e.g., LP+BP) and selectable 12dB, 18dB, and 24dB slopes, available for all filter types.

## v1.1.3 (2025/07/12)
*   Added MOD_C support to the parameter chain and modulation capabilities.

## v1.1.2 (2025/07/08)
*   Added Oscillator update modes allowing various quality and performance modes.

## v1.1.1 (2025/07/06)
*   Added Unilegato functioning both in polyphonic (more than 1 voice) and monophonic (single voice) instances.

## v1.1.0 (2025/07/05)
*   Stable audio generation with full parity to original monolithic version. ADSRs, LFOs, Filter, and Limiter are functional.

## v1.0.0
*   Initial port from monolithic version.
