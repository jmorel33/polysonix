#ifndef POLYSONIX_WAVE_GPU_H
#define POLYSONIX_WAVE_GPU_H

#include "situation.h"

// Rename CPU execution functions to avoid collision and preserve CPU logic as fallback
#define execute_bytecode cpu_execute_bytecode
#define execute_sub_chunk cpu_execute_sub_chunk

// Include the base CPU implementation (AST, Parser, Compiler, CPU VM)
#include "polysonix_wave_cpu.h"

// Undefine so we can provide GPU-aware functions with the original names if needed,
// or just to clean up the namespace.
#undef execute_bytecode
#undef execute_sub_chunk

#ifdef __cplusplus
extern "C" {
#endif

// --- GPU Structures (Match GLSL Layout) ---

typedef struct {
    float x;
    float frequency;
    float rand_offset;
    float modA;
    float modB;
    float modC;
    int32_t lfsr_type;
    uint32_t lfsr_seed;
} VmParamsBuffer;

typedef struct {
    uint32_t main_chunk_offset;
    uint32_t sigma_offsets_0_3[4];
    uint32_t sigma_offsets_4_7[4];
    uint32_t sigma_offsets_8_11[4];
    uint32_t sigma_offsets_12_15[4];
    uint32_t lfsr_periods[4][4];
    uint32_t lfsr_offsets[4][4];
    uint32_t lfsr_tap_masks[4][4];
    uint32_t lfsr_bit_lengths[4][4];
} VmMetadataBuffer;

typedef struct {
    uint64_t vm_params;
    uint64_t vm_metadata;
    uint64_t bytecode;
    uint64_t constants;
    uint64_t lfsr_tables;
    uint64_t output_buffer;
    uint64_t lfsr_state;
} PushConstants;

// --- GPU Globals ---
static SituationComputePipeline wave_compute_pipeline;
static SituationBuffer lfsr_tables_buffer;
static bool gpu_resources_initialized = false;

// --- GPU Helper Functions ---

void init_polysonix_gpu_resources(void) {
    if (gpu_resources_initialized) return;

    if (SituationCreateComputePipeline("polysonix_wave.comp", SITUATION_COMPUTE_LAYOUT_SCALAR, &wave_compute_pipeline) != SITUATION_SUCCESS) {
        fprintf(stderr, "Failed to create compute pipeline for polysonix_wave.comp\n");
        return;
    }

    if (!precomputed_lfsrs[0].initialized) init_polysonix_lfsr_tables();

    size_t total_lfsr_bytes = 0;
    for (int i = 0; i < NUM_LFSR_TYPES; ++i) {
        size_t bytes = LFSR_TABLE_BYTES(precomputed_lfsrs[i].period);
        if (bytes % 4 != 0) bytes += (4 - (bytes % 4));
        total_lfsr_bytes += bytes;
    }

    uint8_t* host_lfsr = (uint8_t*)calloc(1, total_lfsr_bytes);
    if (host_lfsr) {
        size_t offset = 0;
        for (int i = 0; i < NUM_LFSR_TYPES; ++i) {
            size_t bytes = LFSR_TABLE_BYTES(precomputed_lfsrs[i].period);
            if (bytes > 0 && precomputed_lfsrs[i].bit_table) memcpy(host_lfsr + offset, precomputed_lfsrs[i].bit_table, bytes);
            if (bytes % 4 != 0) bytes += (4 - (bytes % 4));
            offset += bytes;
        }
        SituationCreateBuffer(total_lfsr_bytes, host_lfsr, SITUATION_BUFFER_USAGE_STORAGE_BUFFER | SITUATION_BUFFER_USAGE_TRANSFER_DST, &lfsr_tables_buffer);
        free(host_lfsr);
    }
    gpu_resources_initialized = true;
    printf("Polysonix GPU resources initialized.\n");
}

void cleanup_polysonix_gpu_resources(void) {
    if (lfsr_tables_buffer.id != 0) SituationDestroyBuffer(&lfsr_tables_buffer);
    gpu_resources_initialized = false;
}

static size_t serialize_chunk_recursive(BytecodeChunk* chunk, uint8_t* buffer, size_t* current_offset, VmMetadataBuffer* meta, bool is_root) {
    if (!chunk) return 0;

    size_t code_start = *current_offset;
    if (buffer) memcpy(buffer + code_start, chunk->code, chunk->code_count);
    *current_offset += chunk->code_count;

    while ((*current_offset) % 4 != 0) {
        if (buffer) buffer[*current_offset] = OP_HALT;
        (*current_offset)++;
    }

    if (is_root && meta) meta->main_chunk_offset = (uint32_t)code_start;

    for (int i = 0; i < chunk->sigma_sub_chunk_count; ++i) {
        size_t sub_offset = *current_offset;
        if (is_root && meta && i < 16) {
             uint32_t* offsets = meta->sigma_offsets_0_3;
             offsets[i] = (uint32_t)sub_offset;
        }
        serialize_chunk_recursive(chunk->sigma_sub_chunks[i], buffer, current_offset, meta, false);
    }
    return *current_offset;
}

typedef struct {
    SituationBuffer bytecode;
    SituationBuffer constants;
    SituationBuffer metadata;
} GpuWaveBuffers;

GpuWaveBuffers upload_wave_to_gpu(BytecodeChunk* chunk) {
    GpuWaveBuffers gpu_bufs = {0};
    if (!chunk) return gpu_bufs;

    VmMetadataBuffer meta = {0};
    size_t lfsr_offset = 0;
    for (int i = 0; i < NUM_LFSR_TYPES; ++i) {
        meta.lfsr_periods[i/4][i%4] = precomputed_lfsrs[i].period;
        meta.lfsr_offsets[i/4][i%4] = (uint32_t)(lfsr_offset / 4);
        meta.lfsr_tap_masks[i/4][i%4] = lfsr_configs[i].tap_mask;
        meta.lfsr_bit_lengths[i/4][i%4] = lfsr_configs[i].bit_length;
        size_t bytes = LFSR_TABLE_BYTES(precomputed_lfsrs[i].period);
        if (bytes % 4 != 0) bytes += (4 - (bytes % 4));
        lfsr_offset += bytes;
    }

    size_t size = 0;
    serialize_chunk_recursive(chunk, NULL, &size, NULL, true);

    uint8_t* code_bytes = (uint8_t*)calloc(1, size);
    size_t actual_size = 0;
    serialize_chunk_recursive(chunk, code_bytes, &actual_size, &meta, true);

    SituationCreateBuffer(size, code_bytes, SITUATION_BUFFER_USAGE_STORAGE_BUFFER, &gpu_bufs.bytecode);
    SituationCreateBuffer(chunk->constants_count * sizeof(float), chunk->constants, SITUATION_BUFFER_USAGE_STORAGE_BUFFER, &gpu_bufs.constants);
    SituationCreateBuffer(sizeof(VmMetadataBuffer), &meta, SITUATION_BUFFER_USAGE_STORAGE_BUFFER, &gpu_bufs.metadata);

    free(code_bytes);
    return gpu_bufs;
}

// Dispatch compute for a wave (Records command to cmd buffer)
void dispatch_wave_gpu(SituationCommandBuffer cmd, GpuWaveBuffers bufs, VmParams* params, SituationBuffer output_buf, SituationBuffer lfsr_state_buf, SituationBuffer* out_params_buf) {
    if (!gpu_resources_initialized) init_polysonix_gpu_resources();

    VmParamsBuffer pb = {
        .x = params->x, .frequency = params->frequency, .rand_offset = params->rand_offset,
        .modA = params->modA, .modB = params->modB, .modC = params->modC,
        .lfsr_type = (int32_t)params->lfsr_type, .lfsr_seed = params->lfsr_seed
    };

    SituationCreateBuffer(sizeof(VmParamsBuffer), &pb, SITUATION_BUFFER_USAGE_STORAGE_BUFFER, out_params_buf);

    PushConstants pc = {
        .vm_params = SituationGetBufferDeviceAddress(*out_params_buf),
        .vm_metadata = SituationGetBufferDeviceAddress(bufs.metadata),
        .bytecode = SituationGetBufferDeviceAddress(bufs.bytecode),
        .constants = SituationGetBufferDeviceAddress(bufs.constants),
        .lfsr_tables = SituationGetBufferDeviceAddress(lfsr_tables_buffer),
        .output_buffer = SituationGetBufferDeviceAddress(output_buf),
        .lfsr_state = SituationGetBufferDeviceAddress(lfsr_state_buf)
    };

    SituationCmdBindComputePipeline(cmd, wave_compute_pipeline);
    SituationCmdSetPushConstant(cmd, 0, &pc, sizeof(PushConstants));
    SituationCmdDispatch(cmd, 1, 1, 1);
}

// Wrapper to expose standard execute_bytecode (Calls CPU version)
float execute_bytecode(BytecodeChunk *chunk, VmParams* params) {
    return cpu_execute_bytecode(chunk, params);
}

#ifdef __cplusplus
}
#endif

#endif // POLYSONIX_WAVE_GPU_H
