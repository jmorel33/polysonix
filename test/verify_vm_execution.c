#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define PX_VM_IMPLEMENTATION
#include "../px_vm.h"

// Define a test bank of waveforms
#define PX_WAVE_ROM_IMPLEMENTATION
#include "../px_wave_rom.h"

// Mock VmParams
VmParams params;
uint32_t rng_state = 12345;

int main() {
    px_vm_init_lfsr_tables();
    initialize_bytecode_cache();

    // Setup params
    params.x = 0.5f;
    params.frequency = 440.0f;
    params.modA = 0.5f;
    params.modB = 0.5f;
    params.modC = 0.5f;
    params.rand_offset = 0.123f;
    params.lfsr_state = 0xACE1;
    params.lfsr_seed = 0xACE1;
    params.rng_state_ptr = &rng_state;

    // Use index 147 based on grep results
    int patch_id = 147;
    printf("Verifying Patch %d: %s\n", patch_id, default_waves[patch_id].name);
    printf("Expression: %s\n", default_waves[patch_id].expression);

    BytecodeChunk* chunk = compile_expression_to_bytecode(default_waves[patch_id].expression);
    if (!chunk) {
        printf("FAILED to compile patch!\n");
        return 1;
    }
    printf("Compilation Successful. Code count: %d\n", chunk->code_count);

    // Run execution once
    float result = execute_bytecode(chunk, &params);
    printf("Execution Result (x=0.5): %f\n", result);

    if (result == 0.0f) {
        printf("WARNING: Result is exactly 0.0. This might indicate an early exit or error.\n");
    } else {
        printf("Result is non-zero, computation active.\n");
    }

    // Disassemble to check structure
    disassembleChunk(chunk, "Rich String Ensemble");

    return 0;
}
