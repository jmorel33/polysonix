# polysonix.h

A single-header polyphonic synthesizer engine.

<details>
<summary>Table of Contents</summary>

- [Overview](#overview)
- [Key Features](#key-features)
- [Design Principles](#design-principles)
- [CPU vs GPU Backends](#cpu-vs-gpu-backends)
- [Concurrency Model](#concurrency-model)
- [Usage](#usage)
- [Quick Start Example](#quick-start-example)
- [Dependencies](#dependencies)
- [Core API Functions](#core-api-functions)
- [Data Structures](#data-structures)
- [Changelog](#changelog)
- [Polysonix Waveform Scripting Language](#polysonix-waveform-scripting-language)
- [License](#license)

</details>

## Overview
polysonix.h is a flexible, stereo polyphonic synthesizer engine designed to be easily embedded into applications. It is built upon the 'polysonix_wave'
expression engine, allowing for dynamically generated waveforms and complex modulation possibilities via a powerful virtual machine.

This library encapsulates all audio processing and state management, deliberately separating the synthesis core from application-specific logic
like UI, input handling, and audio device management.

## Key Features
- **Thread-Safe by Design:** The library is 100% lock-free. Control functions (like `PX_NoteOn`, `PX_SetFilterParam`) can be safely called from any
  UI or main thread, while the `PX_Process` function runs on the dedicated real-time audio thread.
- **Dynamic Waveform Generation:** Leverages the `polysonix_wave` library to execute bytecode expressions for oscillators and LFOs, enabling complex,
  evolving timbres that go far beyond simple wavetables.
- **Rich Synthesis Architecture:**
  - **Polyphony:** Configurable number of voices (up to 16) with intelligent voice stealing.
  - **ADSR Envelopes**: Up to 3 independent ADSR envelopes per voice for modulating various parameters.
  - **LFOs**: Up to 3 independent Low-Frequency Oscillators (LFOs) with their own ADSRs and flexible routing.
  - **Advanced Multi-Mode Filter:** A highly flexible state-variable filter per voice. It features multiple modes including standard shapes (LP, BP, HP, Notch),
        unique combo-filters (e.g., LP+BP), and a key feature: selectable slopes. These provide distinct tonal characters: 12dB/oct (aggressive, Oberheim-style),
        18dB/oct (balanced, Roland-style), and 24dB/oct (smooth, Moog-style), available for all filter types. The filter also includes key tracking, drive,
        and extensive envelope/LFO modulation.
  - **Unilegato Mode**: Smooth, monophonic legato with pitch sliding between notes.
- **Stereo Signal Path:** Full stereo output with per-voice panning and LFO pan modulation.
- **Built-in Dynamics:** Includes a per-voice soft-clipper to prevent harsh transients and a master bus lookahead limiter to prevent final output clipping.
- **Oscillator Quality Modes**: Choose between per-sample calculation for quality or interpolated modes for performance.
- **Decoupled Design:** The engine is completely independent of any graphics or windowing library. The host application is responsible for the audio
  callback, making the engine portable to any backend (e.g., Raylib, PortAudio, SDL, MiniAudio).

## Design Principles
- **Header-Only:** Designed for easy integration. Simply define `POLYSONIX_IMPLEMENTATION` in one C/C++ file.
- **State Encapsulation:** All synthesizer state is managed within an opaque `PxSynth` handle, ensuring no global state and allowing for multiple
  synth instances if needed.
- **Data-Driven UI:** The library provides a suite of `PX_Get...Info()` functions that return read-only snapshots of the internal state. This allows the host
  application to build a detailed UI without directly accessing internal memory, ensuring a stable and glitch-free API.

## CPU vs GPU Backends

Polysonix offers two distinct backends for waveform generation, allowing developers to choose the best fit for their application's performance profile and platform constraints. By default, the CPU backend is used. To enable the GPU backend, define `POLYSONIX_USE_GPU_WAVE` before including `polysonix.h`.

### CPU Backend (`polysonix_wave_cpu.h`)
The default engine. It is a highly optimized, single-header C library designed for real-time audio generation on the CPU.

*   **Zero Dependencies:** Pure C99, compiles anywhere.
*   **Low Latency:** Instant response to parameter changes, ideal for real-time playing.
*   **Optimized VM:** Uses "Computed Gotos" (on GCC/Clang) and register caching to minimize overhead.
*   **Recursive:** Natural handling of complex nested expressions.

### GPU Backend (`polysonix_wave_gpu.h` + `polysonix_wave.comp`)
An optional backend that offloads synthesis to the graphics card using Compute Shaders. It leverages modern OpenGL 4.6 features for maximum efficiency.

*   **Technology Stack:** Built on **OpenGL 4.6** Compute Shaders.
*   **Bindless Architecture:** Uses **Bindless Descriptors** (via `GL_EXT_buffer_reference2`) to access memory directly, avoiding the overhead of traditional binding slots.
*   **SSBO Storage:** All bytecode, constants, and parameters are managed in **Shader Storage Buffer Objects (SSBOs)** using `GL_EXT_scalar_block_layout` for seamless C-struct compatibility.
*   **Massive Parallelism:** Capable of generating thousands of samples simultaneously.
*   **Feature Parity:** The GLSL shader implements the exact same VM logic (via a non-recursive state machine) as the CPU version.

### Comparative Performance Analysis

The choice between CPU and GPU backends depends largely on the specific requirements of your application, particularly the trade-off between **latency** and **throughput**.

| Feature | CPU Backend | GPU Backend |
| :--- | :--- | :--- |
| **Ideal Use Case** | Real-time interactive synthesis (games, instruments), low-latency audio playback | Offline rendering, audio visualization, massive additive synthesis, texture generation |
| **Latency** | **Negligible** (< 1ms). Direct memory access allows for instant reaction to user input. | **Moderate** (~2-10ms). Requires PCIe transfer, driver dispatch overhead, and read-back synchronization. |
| **Throughput** | **High**. Limited by CPU core count and clock speed. Best for serial processing of independent voices. | **Massive**. Capable of calculating thousands of samples in parallel. Scaling is nearly linear until VRAM bandwidth is saturated. |
| **Complexity Limit** | High complexity (nested loops) can stall the audio thread and cause dropouts. | Can handle extremely complex math and heavy `sigma()` loops without blocking the main CPU thread. |
| **Architecture** | Recursive Stack VM (Uses System Stack) | Iterative State Machine VM (Uses Explicit Stack) |
| **Key Tech** | C99, Computed Gotos | OpenGL 4.6, Bindless SSBOs |

#### Throughput vs. Latency: The Crossover Point

The CPU backend is faster for small-to-medium workloads due to zero dispatch overhead. The GPU backend becomes superior when the sheer volume of calculations (samples * complexity) justifies the fixed cost of communicating with the graphics card.

```text
Performance (Samples/Sec)
      ^
      |                                     / (GPU Backend)
      |                                   /
      |                                 /
      |                               /   <-- Massive Parallelism Scaling
      |                             /
      |---------------------------/-------- (CPU Backend Limit)
      |                         /
      |                       /
      |                     /  <-- Crossover Point (~10k+ concurrent samples)
      |                   /
      |_________________/
      0               Load (Complexity * Polyphony) ->
```

#### Detailed Benchmarks

**CPU Backend:**
Aggressively optimized using computed gotos and register caching.
- **Average Execution Time:** ~100 ns per sample (Apple Silicon M3).
- **Standard Waves:** ~85 ns.
- **Complex/Sigma:** ~200 ns.
- *See [PERFORMANCE_REPORT.md](PERFORMANCE_REPORT.md) for full details.*

**GPU Backend:**
- **Execution Time:** Variable, but effectively "free" for the main CPU thread.
- **Dispatch Cost:** Fixed overhead of ~10-50µs per dispatch depending on driver/OS.
- **Throughput:** Can generate 48kHz audio for 1000+ voices simultaneously in real-time on mid-range hardware.

## Concurrency Model
This library is **THREAD-SAFE**. It uses a lock-free producer/consumer model.
- **Producer (Main/UI Thread):** Calls to control functions like `PX_NoteOn` or `PX_SetFilterParam` do not modify the synth state directly. Instead,
    they place a "command" into a lock-free queue. This is a very fast operation that will not block the UI.
- **Consumer (Audio Thread):** At the beginning of each `PX_Process` call, the audio thread quickly drains the command queue,
    applying all pending state changes in a safe, sequential manner. This ensures the synth's state is perfectly consistent
    for the entire duration of the audio block processing.
- **UI Data:** `Get` functions read from a snapshot of the synth's state, which is safely updated by the audio thread after it finishes processing a block.
    This prevents data races and ensures the UI displays a stable, coherent view of the synthesizer's actual sounding parameters.

## Usage
To use this library, do this in **one** C or C++ file:
```c
#define POLYSONIX_IMPLEMENTATION
#include "polysonix.h"
```
All other files can simply `#include "polysonix.h"`.

1.  Define `POLYSONIX_IMPLEMENTATION` in one C/C++ file before including this header.
2.  Include `polysonix.h` in any other files that need to interact with the synthesizer.
3.  Create a `PxConfig` struct and populate it with your desired settings (sample rate, voices, etc.).
4.  Call `PX_Create()` with your config to get a `PxSynth` instance.
5.  In your audio callback, call `PX_Process()` to generate audio samples.
6.  Use `PX_NoteOn()` and `PX_NoteOff()` from your main thread to control notes.
7.  When finished, call `PX_Destroy()` to free all resources.

## Quick Start Example
```c
// main.c
#include "raylib.h" // Your audio framework
#define POLYSONIX_IMPLEMENTATION
#include "polysonix.h"

int main() {
    // 1. Init audio device and dependencies
    InitAudioDevice();
    polysonix_wave_init();
    // ... compile waveforms ...

    // 2. Configure and create the synth
    PxConfig config = { .num_voices=8, .sample_rate=48000, ... };
    PxSynth* synth = PX_Create(&config);

    // 3. In your main loop, trigger notes and update parameters
    if (IsKeyPressed(KEY_C)) PX_NoteOn(synth, 60, 0, KEY_C);
    if (IsKeyReleased(KEY_C)) PX_NoteOff(synth, KEY_C);

    // 4. In your audio callback/thread, process audio
    // void AudioCallback(void* buffer, unsigned int frames) {
    //     PX_Process(synth, (int16_t*)buffer, frames);
    // }

    // 5. Clean up
    PX_Destroy(synth);
    polysonix_wave_deinit();
    CloseAudioDevice();
}
```

## Dependencies
- **Required:** `polysonix_wave.h` and its implementation must be available  and linked. The host application must call `polysonix_wave_init()`
  before creating a synth instance and is responsible for compiling the waveform expressions used by the synth.

## Core API Functions

The API is designed to be simple and thread-safe.

- `PX_Create(const PxConfig* config)`: Creates and initializes a synthesizer instance.
- `PX_Destroy(PxSynth* s)`: Destroys a synthesizer instance and frees all associated memory.
- `PX_Process(PxSynth* s, int16_t* stereo_buffer, int num_frames)`: Processes a block of audio.
- `PX_NoteOn(PxSynth* s, int midi_note, int wave_idx, int key_id)`: Triggers a new note.
- `PX_NoteOff(PxSynth* s, int key_id)`: Releases a note.

The library also provides a comprehensive set of `PX_Set...` and `PX_Get...` functions for controlling all aspects of the synthesizer, including:

- Voice ADSR parameters
- LFO parameters and routing
- Filter parameters
- Global settings like pan and limiter
- Unilegato settings

Additionally, there are several `PX_Get...Info()` functions that provide read-only snapshots of the internal state for UI display.

## Data Structures

- `PxSynth`: An opaque handle representing the synthesizer instance.
- `PxConfig`: A struct for configuring the synthesizer upon creation.
- `PxPatch`: A struct that holds the editable parameters of the sound.
- `PxVoiceInfo`: A read-only snapshot of a voice's real-time state.
- `PxLimiterInfo`: A read-only snapshot of the limiter's state.
- `PxWaveInfo`: Information about a specific waveform.
- `PxLFOInfo`: A read-only snapshot of an LFO's state.
- `PxADSRParams`: A struct for defining ADSR envelope parameters.
- `PxLFOParams`: A struct for defining LFO parameters.

## Changelog
For the full history of changes, please see [updatelog.txt](updatelog.txt).

## Polysonix Waveform Scripting Language
The Polysonix Waveform Scripting Language is a domain-specific language for defining mathematical expressions that generate audio waveforms in the Polysonix synthesizer. Expressions are stored as strings, tokenized, parsed into an abstract syntax tree (AST), compiled into bytecode, and executed by a virtual machine (VM) for real-time audio synthesis.

### Language Structure
The language follows a C-like mathematical expression grammar. Expressions are composed of:

- **Literals**: Floating-point numbers (e.g., `1.0`, `0.5`).
- **Variables**: `x`, `FREQUENCY`, `MOD_A`, `MOD_B`, `MOD_C`, `RAND_OFFSET`, and loop variables (e.g., `k`).
- **Constants**: `PI`, `TWO_PI`, `PI_OVER_2`, `THREE_PI_OVER_2`, `E`, LFSR type constants.
- **Functions**: Mathematical, utility, and LFSR functions (e.g., `sin`, `sigma`, `lfsr_val`).
- **Operators**: Arithmetic, unary, comparison, logical, and ternary.
- **Grouping**: Parentheses `()` for precedence and function arguments.
- **Commas**: Separate function arguments.

### Operand Symbols
The language supports the following operators:

- **Arithmetic (Binary)**: `+`, `-`, `*`, `/`, `%`
- **Unary**: `+`, `-`, `!`
- **Comparison**: `<`, `>`, `<=`, `>=`, `==`, `!=`
- **Logical**: `&&`, `||`, `^`
- **Ternary**: `?`, `:`
- **Other**: `()`, `,`

### Variables and Parameters
- `x`: Phase, typically normalized from 0 to 2*PI over one wave cycle.
- `FREQUENCY`: Current note frequency in Hertz (Hz).
- `MOD_A`, `MOD_B`, `MOD_C`: Modulation parameters, typically ranging from -1.0 to 1.0.
- `RAND_OFFSET`: A per-wave random value, typically from 0.0 to 1.0, constant for the duration of one wave generation.
- `k`: Default loop variable name for the `sigma()` summation function.

### Constants
- `PI`, `TWO_PI`, `PI_OVER_2`, `THREE_PI_OVER_2`, `E`
- **LFSR Types**: `LFSR_4BIT` through `LFSR_17BIT`, `LFSR_GALOIS`, `LFSR_FIBONACCI`

### Functions
- **Trigonometric**: `sin`, `cos`, `tan`, `asin`, `acos`, `atan`
- **Numeric**: `abs`, `tanh`, `exp`, `log`, `log10`, `floor`, `ceil`, `min`, `max`, `sqrt`, `pow`, `rand`
- **Summation**: `sigma(k, start, end, step, expr)`
- **LFSR**: `lfsr_val`, `lfsr_noise`, `lfsr_clock`

### Examples

**Basic Sawtooth Wave**
```
1.0 - (x / PI)
```

**Pulse Width Modulation (PWM)**
```
x < (PI + MOD_A * PI) ? 1.0 : -1.0
```

**Additive Synthesis with Sigma**
```
sigma(k, 1.0, 8.0, 1.0, sin(x*k)/k)
```

**LFSR-based Noise**
```
lfsr_noise(LFSR_8BIT, 2.0 + MOD_B)
```

## License
polysonix.h is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
