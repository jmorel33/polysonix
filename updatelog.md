# Polysonix Update Log

## v1.0Alpha1

This release seals the current codebase as **Version 1.0Alpha1**, marking a significant milestone in stability and mathematical precision.

### Fixes
*   **ADSR Envelope Accuracy:** Corrected the decay and release calculations in `ADSR_Update`. Replaced the linear single-step multiplier application with an exponential `powf` calculation based on the actual sample count (`num_steps`). This fixes issues where envelopes played slower than intended during block-based updates.
*   **Interpolation Phase Continuity:** Improved the phase tracking logic in `PX_Process` for oscillator interpolation. The start phase for a new processing block is now preserved from the previous block's end phase (`phase_at_interp_end`), rather than being back-calculated from the current frequency. This ensures seamless audio continuity even under heavy frequency modulation.
*   **LFO Synchronization:** Fixed a bug in `PX_NoteOn_internal` where LFOs configured to not reset on key press (`reset_on_key_on = false`) failed to synchronize with the global LFO state. These LFOs now correctly inherit the phase and VM state from the master `template_lfo_instances`, ensuring true free-running behavior across voice reuse.

## v1.0Alpha0

### Fixes
*   **Waveform Generation Precision:** In `polysonix_wave`, fixed an issue where CPU rendering of waveforms was previously quantized to 16-bit integers. This has been updated to use 32-bit floating-point precision, eliminating quantization artifacts and significantly improving audio fidelity.
