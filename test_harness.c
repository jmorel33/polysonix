#define POLYSONIX_IMPLEMENTATION
#include "polysonix.h"
#include "px_vm_bank.h" // Provides default_waves definition

#include <stdio.h>
#include <math.h>
#include <assert.h>

#define TEST_SAMPLE_RATE 44100.0f
#define TEST_BLOCK_SIZE 256

void test_bytecodes(PxSynth* synth) {
    printf("\n--- Testing 256 Bytecode Waveforms ---\n");
    float buffer[TEST_BLOCK_SIZE * 2];
    int num_waveforms = PX_GetNumWaveforms();

    for (int i = 0; i < num_waveforms; ++i) {
        PxWaveInfo info = PX_GetWaveInfo(i);
        printf("Testing Wave %d: %s... ", i, info.name);
        fflush(stdout);

        // 1. Set Oscillator 1 to this wave
        PX_SetOscWave(synth, 0, i);
        PX_SetOscEnabled(synth, 0, true);
        PX_SetOscMix(synth, 0, 1.0f);

        // Disable WSEQ for this test
        PX_SetOscSequence(synth, 0, -1);

        // 2. Trigger Note
        int key_id = 1000 + i;
        PX_NoteOn(synth, 60, i, key_id, 0.8f);

        // 3. Process a few blocks
        bool silent = true;
        bool valid = true;

        for (int b = 0; b < 10; ++b) {
            PX_Process(synth, buffer, TEST_BLOCK_SIZE);
            for (int s = 0; s < TEST_BLOCK_SIZE * 2; ++s) {
                if (fabsf(buffer[s]) > 0.0001f) silent = false;
                if (isnan(buffer[s]) || isinf(buffer[s])) {
                    valid = false;
                    break;
                }
            }
            if (!valid) break;
        }

        PX_NoteOff(synth, key_id);

        // Allow release phase
        PX_Process(synth, buffer, TEST_BLOCK_SIZE);

        if (!valid) {
            printf("FAILED (NaN/Inf detected)\n");
        } else if (silent) {
            // Some waves might legitimately be silent or very quiet depending on mod params
            printf("WARNING (Silence detected)\n");
        } else {
            printf("OK\n");
        }
    }
}

void test_sequences(PxSynth* synth) {
    printf("\n--- Testing 256 Wave Sequences ---\n");
    float buffer[TEST_BLOCK_SIZE * 2];

    for (int i = 0; i < PX_NUM_WSEQ_BANKS; ++i) {
        // Access internal ROM name if possible
        const char* seq_name = ROM_WAVE_SEQUENCES[i].name;
        printf("Testing Sequence %d: %s... ", i, seq_name);
        fflush(stdout);

        // 1. Set Osc 1 to use Sequence i
        PX_SetOscSequence(synth, 0, i);
        PX_SetOscEnabled(synth, 0, true);

        // 2. Trigger Note
        int key_id = 2000 + i;
        PX_NoteOn(synth, 60, 0, key_id, 0.8f);

        // 3. Process
        bool valid = true;

        // Process enough frames
        for (int b = 0; b < 20; ++b) {
            PX_Process(synth, buffer, TEST_BLOCK_SIZE);

            // Validate
             for (int s = 0; s < TEST_BLOCK_SIZE * 2; ++s) {
                if (isnan(buffer[s]) || isinf(buffer[s])) {
                    valid = false;
                    break;
                }
            }
            if (!valid) break;
        }

        PX_NoteOff(synth, key_id);
        PX_Process(synth, buffer, TEST_BLOCK_SIZE); // Process release

        if (!valid) {
            printf("FAILED (NaN/Inf)\n");
        } else {
            printf("OK\n");
        }
        fflush(stdout);
    }
}

int main() {
    printf("Polysonix Test Harness (Full)\n");

    PxConfig config = {
        .num_voices = 4,
        .num_lfos = 2,
        .num_voice_adsrs = 2,
        .sample_rate = TEST_SAMPLE_RATE,
        .osc_update_mode = PX_OSC_UPDATE_MODE_FIXED_RATE,
        .osc_fixed_update_rate_hz = 35000.0f,
        .use_gpu = false
    };

    PxSynth* synth = PX_Create(&config);
    if (!synth) {
        fprintf(stderr, "Failed to create synth instance.\n");
        return 1;
    }

    // Compile ALL waves first to ensure VM is active
    printf("Compiling all bytecodes...\n");
    for (int i = 0; i < NUM_DEFAULT_WAVES; ++i) {
        if (!get_default_wave_bytecode(i)) {
            printf("Failed to compile wave %d\n", i);
        }
    }
    printf("Compilation complete.\n");

    test_bytecodes(synth);
    test_sequences(synth);

    PX_Destroy(synth);
    printf("\nTest Harness Completed.\n");
    return 0;
}
