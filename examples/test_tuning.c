#define POLYSONIX_IMPLEMENTATION
#include "polysonix_wave_bank.h" // Includes polysonix_wave.h and defines default_waves
#include "../polysonix.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

void test_tuning() {
    printf("Starting Tuning Test...\n");

    PxConfig config = {
        .num_voices = 4,
        .num_lfos = 2,
        .num_voice_adsrs = 2,
        .sample_rate = 44100.0f,
        .samples_per_lfo_update = 64,
        .lfo_update_interval_ms = 1.0f,
        .osc_update_mode = PX_OSC_UPDATE_MODE_PER_SAMPLE,
        .osc_fixed_update_rate_hz = 0.0f,
        .nyquist_precision_multiplier = 0.0f,
        .use_gpu = false
    };

    PxSynth* synth = PX_Create(&config);
    if (!synth) {
        fprintf(stderr, "Failed to create synth\n");
        return;
    }

    // Mock bytecode so it doesn't crash if it tries to execute
    // Actually PX_Process checks for NULL bytecode but let's be safe or just rely on the fact we are testing frequency calculation which happens before bytecode execution in most cases or is independent.
    // The loop uses default_waves[v->source_wave_index].compiled_bytecode.
    // Since we are compiling implementation here, default_waves is available?
    // Wait, polysonix_wave.h is included. It probably needs `examples/polysonix_wave_bank.h` if POLYSONIX_IMPLEMENTATION is defined.
    // Let's check polysonix.h includes polysonix_wave.h.
    // polysonix_wave.h likely needs the bank.

    float buffer[128];
    int wave_idx = 0;
    int midi_note = 69; // A4 = 440Hz

    // --- Test 1: Default Tuning (0, 0) ---
    printf("Test 1: Default Tuning\n");
    PX_NoteOn(synth, midi_note, wave_idx, 1, 1.0f);
    PX_Process(synth, buffer, 64);

    // Internal inspection since I defined IMPLEMENTATION
    Voice* v = &synth->voices[0];
    printf("Voice 0 Frequency: %.2f Hz (Expected 440.00)\n", v->frequency);
    assert(fabs(v->frequency - 440.0f) < 0.1f);

    PX_NoteOff(synth, 1);
    PX_Process(synth, buffer, 64); // Process release

    // --- Test 2: Coarse Tuning +12 Semitones (Octave Up) ---
    printf("Test 2: Coarse +12 Semitones\n");
    PX_SetOscCoarseTune(synth, wave_idx, 12.0f);
    PX_NoteOn(synth, midi_note, wave_idx, 2, 1.0f);
    PX_Process(synth, buffer, 64);

    v = &synth->voices[0]; // Assuming voice 0 is picked again or we find active one.
    // Find active voice
    for(int i=0; i<4; i++) {
        if (synth->voices[i].active && synth->voices[i].key_id == 2) {
            v = &synth->voices[i];
            break;
        }
    }

    printf("Voice Frequency: %.2f Hz (Expected 880.00)\n", v->frequency);
    assert(fabs(v->frequency - 880.0f) < 0.2f);

    PX_NoteOff(synth, 2);
    PX_Process(synth, buffer, 64);

    // --- Test 3: Fine Tuning +50 Cents ---
    printf("Test 3: Fine +50 Cents (Coarse still +12)\n");
    PX_SetOscFineTune(synth, wave_idx, 50.0f);
    PX_NoteOn(synth, midi_note, wave_idx, 3, 1.0f);
    PX_Process(synth, buffer, 64);

    for(int i=0; i<4; i++) {
        if (synth->voices[i].active && synth->voices[i].key_id == 3) {
            v = &synth->voices[i];
            break;
        }
    }

    float expected_freq = 440.0f * powf(2.0f, (12.0f + 0.5f) / 12.0f);
    printf("Voice Frequency: %.2f Hz (Expected %.2f)\n", v->frequency, expected_freq);
    assert(fabs(v->frequency - expected_freq) < 0.5f);

    PX_Destroy(synth);
    printf("Test Passed!\n");
}

int main() {
    test_tuning();
    return 0;
}
