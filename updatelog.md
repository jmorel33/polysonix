# Polysonix Update Log

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

## v1.1.7 (20256/01/03)

### Fixes
*   **Waveform Generation Precision:** In `polysonix_wave`, fixed an issue where CPU rendering of waveforms was previously quantized to 16-bit integers. This has been updated to use 32-bit floating-point precision, eliminating quantization artifacts and significantly improving audio fidelity.
