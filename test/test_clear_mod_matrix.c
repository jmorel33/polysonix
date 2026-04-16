#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#define POLYSONIX_IMPLEMENTATION
#include "polysonix.h"

int main() {
    printf("Testing PX_ClearModMatrix...\n");

    PxConfig config = {0};
    config.num_voices = 16;
    config.num_lfos = 3;
    config.num_voice_adsrs = 3;
    config.sample_rate = 44100.0f;
    config.lfo_update_interval_ms = 1.0f;
    config.osc_update_mode = PX_OSC_UPDATE_MODE_PER_SAMPLE;

    PxSynth* s = PX_Create(&config);
    assert(s != NULL);

    // Initial state: all slots should be disabled (based on PX_Create implementation)
    for (int i = 0; i < PX_MOD_MATRIX_SLOTS; i++) {
        assert(s->patch.mod_matrix[i].enabled == false);
    }

    // Set some slots
    PX_SetModMatrixSlot(s, 0, PX_MOD_SRC_MODWHEEL, PX_MOD_DEST_FILTER_CUTOFF, 0.5f);
    PX_EnableModMatrixSlot(s, 0, true);
    PX_SetModMatrixSlot(s, 5, PX_MOD_SRC_VELOCITY, PX_MOD_DEST_OSC_MODA, -0.8f);
    PX_EnableModMatrixSlot(s, 5, true);

    // Process commands
    float dummy_buffer[256];
    PX_Process(s, dummy_buffer, 128);

    // Verify they are set
    assert(s->patch.mod_matrix[0].enabled == true);
    assert(s->patch.mod_matrix[0].amount == 0.5f);
    assert(s->patch.mod_matrix[0].source == PX_MOD_SRC_MODWHEEL);
    assert(s->patch.mod_matrix[0].dest == PX_MOD_DEST_FILTER_CUTOFF);

    assert(s->patch.mod_matrix[5].enabled == true);
    assert(s->patch.mod_matrix[5].amount == -0.8f);
    assert(s->patch.mod_matrix[5].source == PX_MOD_SRC_VELOCITY);
    assert(s->patch.mod_matrix[5].dest == PX_MOD_DEST_OSC_MODA);

    // Clear the matrix
    PX_ClearModMatrix(s);

    // Process the clear command
    PX_Process(s, dummy_buffer, 128);

    // Verify all slots are cleared
    for (int i = 0; i < PX_MOD_MATRIX_SLOTS; i++) {
        if (s->patch.mod_matrix[i].enabled != false || s->patch.mod_matrix[i].amount != 0.0f) {
            printf("Slot %d not cleared: enabled=%d, amount=%f\n", i, s->patch.mod_matrix[i].enabled, s->patch.mod_matrix[i].amount);
            assert(false);
        }
    }

    PX_Destroy(s);

    printf("PX_ClearModMatrix test passed!\n");
    return 0;
}
