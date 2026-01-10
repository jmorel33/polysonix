#ifndef PX_PATCHING_H
#define PX_PATCHING_H

#include "polysonix.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- IO Abstraction ---

/**
 * @brief Callback for writing a batch of data to a stream/bus.
 * @param token User-provided context.
 * @param data Pointer to the data to write.
 * @param size Number of bytes to write.
 * @return Number of bytes actually written.
 */
typedef size_t (*PxIOWriteFn)(void* token, const void* data, size_t size);

/**
 * @brief Callback for reading a batch of data from a stream/bus.
 * @param token User-provided context.
 * @param data Pointer to the buffer to read into.
 * @param size Number of bytes to read.
 * @return Number of bytes actually read.
 */
typedef size_t (*PxIOReadFn)(void* token, void* data, size_t size);

// --- Core API ---

/**
 * @brief Calculates the total binary size (Header + Data + Checksum) required for a preset.
 * @param s The synthesizer instance.
 * @return The size in bytes.
 */
PX_API size_t PX_CalculatePresetSize(PxSynth* s);

/**
 * @brief Saves the current patch to an abstract bus/stream using a single batched write.
 * @details This function allocates a temporary buffer, serializes the full preset (Header, Data, Checksum),
 *          and calls the write_fn exactly once with the complete payload.
 * @param s The synthesizer instance.
 * @param write_fn The callback to handle the write operation.
 * @param token Context passed to the callback.
 * @param patch_name Name of the patch (max 15 chars).
 * @return true on success, false on failure.
 */
PX_API bool PX_SavePresetToBus(PxSynth* s, PxIOWriteFn write_fn, void* token, const char* patch_name);

/**
 * @brief Loads a patch from an abstract bus/stream.
 * @param s The synthesizer instance.
 * @param read_fn The callback to handle read operations.
 * @param token Context passed to the callback.
 * @return true on success, false on failure.
 */
PX_API bool PX_LoadPresetFromBus(PxSynth* s, PxIOReadFn read_fn, void* token);

// --- File IO Wrappers ---

/**
 * @brief Saves the current patch to a file (disk).
 * @param s The synthesizer instance.
 * @param filename File path.
 * @param patch_name Patch name.
 * @return true on success.
 */
PX_API bool PX_SavePreset(PxSynth* s, const char* filename, const char* patch_name);

/**
 * @brief Loads a patch from a file (disk).
 * @param s The synthesizer instance.
 * @param filename File path.
 * @return true on success.
 */
PX_API bool PX_LoadPreset(PxSynth* s, const char* filename);

// --- Patch Bank API ---

#define PX_PATCH_BANK_SIZE 128

/**
 * @struct PxPatchBank
 * @brief A container for multiple patches, matching a specific configuration.
 */
typedef struct PxPatchBank {
    PxConfig config;
    PxPatch* patches; // Array of size PX_PATCH_BANK_SIZE
} PxPatchBank;

/**
 * @brief Creates a new patch bank initialized with the given configuration.
 * @details Allocates memory for the bank and all internal structures (ADSRs, LFOs) for each patch slot.
 * @param config The configuration to use for allocating patch memory.
 * @return A pointer to the new bank, or NULL on failure.
 */
PX_API PxPatchBank* PX_CreatePatchBank(const PxConfig* config);

/**
 * @brief Destroys a patch bank and frees all associated memory.
 * @param bank The bank to destroy.
 */
PX_API void PX_DestroyPatchBank(PxPatchBank* bank);

/**
 * @brief Saves the current state of the synth to a specific slot in the bank.
 * @param bank The destination bank.
 * @param slot_idx The slot index (0 to PX_PATCH_BANK_SIZE - 1).
 * @param s The source synthesizer.
 * @return true on success, false if invalid index or bank/synth is NULL.
 */
PX_API bool PX_Bank_SaveToSlot(PxPatchBank* bank, int slot_idx, PxSynth* s);

/**
 * @brief Loads a patch from a specific slot in the bank to the synth.
 * @param bank The source bank.
 * @param slot_idx The slot index (0 to PX_PATCH_BANK_SIZE - 1).
 * @param s The destination synthesizer.
 * @return true on success, false if invalid index or bank/synth is NULL.
 */
PX_API bool PX_Bank_LoadFromSlot(PxPatchBank* bank, int slot_idx, PxSynth* s);

/**
 * @brief Copies a patch from one slot to another within the same bank.
 * @param bank The bank.
 * @param src_idx Source slot index.
 * @param dest_idx Destination slot index.
 * @return true on success.
 */
PX_API bool PX_Bank_CopySlot(PxPatchBank* bank, int src_idx, int dest_idx);

#ifdef __cplusplus
}
#endif

#endif // PX_PATCHING_H

#ifdef POLYSONIX_PATCHING_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Helper Functions ---

static void write_be32_to_buf(uint8_t* buf, uint32_t val) {
    buf[0] = (uint8_t)((val >> 24) & 0xFF);
    buf[1] = (uint8_t)((val >> 16) & 0xFF);
    buf[2] = (uint8_t)((val >> 8) & 0xFF);
    buf[3] = (uint8_t)(val & 0xFF);
}

static uint32_t read_be32_from_buf(const uint8_t* buf) {
    return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
}

// --- Serialization Core ---

static void px_serialize_patch_impl(const PxPatch* p, const PxConfig* c, uint8_t** ptr_ref, size_t* size_ref, bool calculate_only) {
    uint8_t* ptr = ptr_ref ? *ptr_ref : NULL;
    size_t size = 0;

    #define SER_BUF(data, data_size) \
        do { \
            if (!calculate_only) { if(ptr) memcpy(ptr, data, data_size); ptr += data_size; } \
            size += data_size; \
        } while(0)

    #define SER_SCALAR(val, type) SER_BUF(&val, sizeof(type))

    if (p->template_voice_adsrs)
        SER_BUF(p->template_voice_adsrs, sizeof(PxADSRParams) * c->num_voice_adsrs);
    if (p->template_voice_adsr_mod_amounts)
        SER_BUF(p->template_voice_adsr_mod_amounts, sizeof(float) * c->num_voice_adsrs * PX_ADSR_DEST_COUNT);
    if (p->template_lfos)
        SER_BUF(p->template_lfos, sizeof(PxLFOParams) * c->num_lfos);

    SER_SCALAR(p->filter_cutoff_hz, float);
    SER_SCALAR(p->filter_resonance_q, float);
    SER_SCALAR(p->filter_env_amount_hz, float);
    SER_SCALAR(p->filter_drive, float);
    SER_SCALAR(p->filter_key_track, float);
    SER_SCALAR(p->filter_poles, int);
    SER_SCALAR(p->filter_mode, PxFilterMode);
    SER_SCALAR(p->voice_pan_setting, float);
    SER_SCALAR(p->default_note_amp, float);
    SER_SCALAR(p->limiter_threshold, float);
    SER_SCALAR(p->limiter_release_ms, float);
    SER_SCALAR(p->unilegato_enabled, bool);
    SER_SCALAR(p->unilegato_slide_duration_s, float);
    SER_BUF(p->mod_matrix, sizeof(PxModSlot) * PX_MOD_MATRIX_SLOTS);
    SER_SCALAR(p->pitchbend_range_semitones, float);
    SER_SCALAR(p->global_filter_enabled, bool);
    SER_SCALAR(p->global_filter_cutoff_hz, float);
    SER_SCALAR(p->global_filter_resonance_q, float);
    SER_SCALAR(p->global_filter_env_amount_hz, float);
    SER_SCALAR(p->global_filter_drive, float);
    SER_SCALAR(p->global_filter_key_track, float);
    SER_SCALAR(p->global_filter_poles, int);
    SER_SCALAR(p->global_filter_mode, PxFilterMode);
    SER_SCALAR(p->velocity_curve, PxCurveType);
    SER_SCALAR(p->aftertouch_curve, PxCurveType);
    SER_BUF(p->osc, sizeof(PxOscillator) * PX_MAX_OSC_PER_VOICE);

    #undef SER_BUF
    #undef SER_SCALAR

    if (ptr_ref && !calculate_only) *ptr_ref = ptr;
    if (size_ref) *size_ref = size;
}

static bool px_deserialize_patch_impl(PxPatch* p, const PxConfig* c, const uint8_t** ptr_ref, const uint8_t* end) {
    const uint8_t* ptr = *ptr_ref;

    #define DESER_BUF(data, data_size) \
        do { \
            if (ptr + data_size > end) return false; \
            memcpy(data, ptr, data_size); \
            ptr += data_size; \
        } while(0)

    #define DESER_SCALAR(val, type) DESER_BUF(&val, sizeof(type))

    if (p->template_voice_adsrs)
        DESER_BUF(p->template_voice_adsrs, sizeof(PxADSRParams) * c->num_voice_adsrs);
    if (p->template_voice_adsr_mod_amounts)
        DESER_BUF(p->template_voice_adsr_mod_amounts, sizeof(float) * c->num_voice_adsrs * PX_ADSR_DEST_COUNT);
    if (p->template_lfos)
        DESER_BUF(p->template_lfos, sizeof(PxLFOParams) * c->num_lfos);

    DESER_SCALAR(p->filter_cutoff_hz, float);
    DESER_SCALAR(p->filter_resonance_q, float);
    DESER_SCALAR(p->filter_env_amount_hz, float);
    DESER_SCALAR(p->filter_drive, float);
    DESER_SCALAR(p->filter_key_track, float);
    DESER_SCALAR(p->filter_poles, int);
    DESER_SCALAR(p->filter_mode, PxFilterMode);
    DESER_SCALAR(p->voice_pan_setting, float);
    DESER_SCALAR(p->default_note_amp, float);
    DESER_SCALAR(p->limiter_threshold, float);
    DESER_SCALAR(p->limiter_release_ms, float);
    DESER_SCALAR(p->unilegato_enabled, bool);
    DESER_SCALAR(p->unilegato_slide_duration_s, float);
    DESER_BUF(p->mod_matrix, sizeof(PxModSlot) * PX_MOD_MATRIX_SLOTS);
    DESER_SCALAR(p->pitchbend_range_semitones, float);
    DESER_SCALAR(p->global_filter_enabled, bool);
    DESER_SCALAR(p->global_filter_cutoff_hz, float);
    DESER_SCALAR(p->global_filter_resonance_q, float);
    DESER_SCALAR(p->global_filter_env_amount_hz, float);
    DESER_SCALAR(p->global_filter_drive, float);
    DESER_SCALAR(p->global_filter_key_track, float);
    DESER_SCALAR(p->global_filter_poles, int);
    DESER_SCALAR(p->global_filter_mode, PxFilterMode);
    DESER_SCALAR(p->velocity_curve, PxCurveType);
    DESER_SCALAR(p->aftertouch_curve, PxCurveType);
    DESER_BUF(p->osc, sizeof(PxOscillator) * PX_MAX_OSC_PER_VOICE);

    #undef DESER_BUF
    #undef DESER_SCALAR

    *ptr_ref = ptr;
    return true;
}

PX_API size_t PX_CalculatePresetSize(PxSynth* s) {
    size_t size = 32; // Header
    size_t body_size = 0;
    px_serialize_patch_impl(&s->patch, &s->config, NULL, &body_size, true);
    size += body_size;
    size += 4; // Footer Checksum
    return size;
}

PX_API bool PX_SavePresetToBus(PxSynth* s, PxIOWriteFn write_fn, void* token, const char* patch_name) {
    if (!s || !write_fn) return false;

    size_t total_size = PX_CalculatePresetSize(s);
    uint8_t* buffer = (uint8_t*)malloc(total_size);
    if (!buffer) return false;

    uint8_t* ptr = buffer;

    // --- Header ---
    memcpy(ptr, "POLY", 4); ptr += 4;
    uint8_t version[4] = {POLYSONIX_VERSION_MAJOR, POLYSONIX_VERSION_MINOR, POLYSONIX_VERSION_PATCH, 0};
    memcpy(ptr, version, 4); ptr += 4;

    char name[16];
    memset(name, ' ', 16);
    if (patch_name) {
        size_t len = strlen(patch_name);
        if (len > 16) len = 16;
        memcpy(name, patch_name, len);
    }
    memcpy(ptr, name, 16); ptr += 16;

    // Data Length (Total - Header - Footer)
    uint32_t data_len = (uint32_t)(total_size - 32 - 4);
    write_be32_to_buf(ptr, data_len); ptr += 4;

    // Reserved
    memset(ptr, 0, 4); ptr += 4;

    // --- Data Block ---
    uint8_t* data_start = ptr;
    px_serialize_patch_impl(&s->patch, &s->config, &ptr, NULL, false);

    // --- Checksum ---
    uint32_t checksum = 0;
    size_t written_data_len = (size_t)(ptr - data_start);
    for (size_t i = 0; i < written_data_len; i++) {
        checksum += data_start[i];
    }

    write_be32_to_buf(ptr, checksum); ptr += 4;

    // --- Batch Write ---
    size_t written = write_fn(token, buffer, total_size);

    free(buffer);
    return (written == total_size);
}

PX_API bool PX_LoadPresetFromBus(PxSynth* s, PxIOReadFn read_fn, void* token) {
    if (!s || !read_fn) return false;

    // 1. Read Header
    uint8_t header[32];
    if (read_fn(token, header, 32) != 32) return false;

    if (strncmp((char*)header, "POLY", 4) != 0) return false;
    // Version check (strict major/minor)
    if (header[4] != POLYSONIX_VERSION_MAJOR || header[5] != POLYSONIX_VERSION_MINOR) return false;

    uint32_t data_len = read_be32_from_buf(header + 24);

    // 2. Allocate Data + Footer
    size_t payload_size = data_len + 4;
    uint8_t* buffer = (uint8_t*)malloc(payload_size);
    if (!buffer) return false;

    // 3. Read Body + Checksum
    if (read_fn(token, buffer, payload_size) != payload_size) {
        free(buffer); return false;
    }

    // 4. Verify Checksum
    uint32_t expected_checksum = read_be32_from_buf(buffer + data_len);
    uint32_t calc_sum = 0;
    for (uint32_t i = 0; i < data_len; i++) calc_sum += buffer[i];

    if (calc_sum != expected_checksum) {
        free(buffer); return false;
    }

    // 5. Deserialize
    const uint8_t* ptr = buffer;
    const uint8_t* end = buffer + data_len;

    bool result = px_deserialize_patch_impl(&s->patch, &s->config, &ptr, end);

    free(buffer);
    return result;
}

// --- Wrappers for File IO ---

static size_t file_write_wrapper(void* token, const void* data, size_t size) {
    return fwrite(data, 1, size, (FILE*)token);
}

static size_t file_read_wrapper(void* token, void* data, size_t size) {
    return fread(data, 1, size, (FILE*)token);
}

PX_API bool PX_SavePreset(PxSynth* s, const char* filename, const char* patch_name) {
    FILE* f = fopen(filename, "wb");
    if (!f) return false;
    bool res = PX_SavePresetToBus(s, file_write_wrapper, f, patch_name);
    fclose(f);
    return res;
}

PX_API bool PX_LoadPreset(PxSynth* s, const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return false;
    bool res = PX_LoadPresetFromBus(s, file_read_wrapper, f);
    fclose(f);
    return res;
}

// --- Patch Bank Implementation ---

// Helper: Deep copy a single patch
static bool px_copy_patch_deep(PxPatch* dest, const PxPatch* src, const PxConfig* config) {
    // Debug
    // printf("Copying Deep: ADSR Count: %d\n", config->num_voice_adsrs);
    // if (dest->template_voice_adsrs) printf("Dest ADSR Ptr Before: %p\n", dest->template_voice_adsrs);
    // if (src->template_voice_adsrs) printf("Src ADSR Ptr: %p\n", src->template_voice_adsrs);

    // Save dest pointers (preservation of allocated memory)
    PxADSRParams* dest_adsrs = dest->template_voice_adsrs;
    float* dest_mod_amounts = dest->template_voice_adsr_mod_amounts;
    PxLFOParams* dest_lfos = dest->template_lfos;

    // Apply shallow copy (copies all scalars and embedded arrays)
    *dest = *src;

    // Restore dest pointers (so we don't leak memory or lose our buffers)
    dest->template_voice_adsrs = dest_adsrs;
    dest->template_voice_adsr_mod_amounts = dest_mod_amounts;
    dest->template_lfos = dest_lfos;

    // Perform deep copy into buffers
    if (dest->template_voice_adsrs && src->template_voice_adsrs)
        memcpy(dest->template_voice_adsrs, src->template_voice_adsrs, sizeof(PxADSRParams) * config->num_voice_adsrs);

    if (dest->template_voice_adsr_mod_amounts && src->template_voice_adsr_mod_amounts)
        memcpy(dest->template_voice_adsr_mod_amounts, src->template_voice_adsr_mod_amounts, sizeof(float) * config->num_voice_adsrs * PX_ADSR_DEST_COUNT);

    if (dest->template_lfos && src->template_lfos)
        memcpy(dest->template_lfos, src->template_lfos, sizeof(PxLFOParams) * config->num_lfos);

    return true;
}

// Helper: Allocate patch memory
static bool px_allocate_patch_memory(PxPatch* p, const PxConfig* config) {
    p->template_voice_adsrs = (PxADSRParams*)calloc(config->num_voice_adsrs, sizeof(PxADSRParams));
    p->template_voice_adsr_mod_amounts = (float*)calloc(config->num_voice_adsrs * PX_ADSR_DEST_COUNT, sizeof(float));
    p->template_lfos = (PxLFOParams*)calloc(config->num_lfos, sizeof(PxLFOParams));

    if (!p->template_voice_adsrs || !p->template_voice_adsr_mod_amounts || !p->template_lfos) {
        if (p->template_voice_adsrs) free(p->template_voice_adsrs);
        if (p->template_voice_adsr_mod_amounts) free(p->template_voice_adsr_mod_amounts);
        if (p->template_lfos) free(p->template_lfos);
        return false;
    }
    return true;
}

static void px_free_patch_memory(PxPatch* p) {
    if (p->template_voice_adsrs) free(p->template_voice_adsrs);
    if (p->template_voice_adsr_mod_amounts) free(p->template_voice_adsr_mod_amounts);
    if (p->template_lfos) free(p->template_lfos);
}

PX_API PxPatchBank* PX_CreatePatchBank(const PxConfig* config) {
    if (!config) return NULL;

    PxPatchBank* bank = (PxPatchBank*)calloc(1, sizeof(PxPatchBank));
    if (!bank) return NULL;

    bank->config = *config;
    bank->patches = (PxPatch*)calloc(PX_PATCH_BANK_SIZE, sizeof(PxPatch));
    if (!bank->patches) {
        free(bank);
        return NULL;
    }

    // Allocate deep memory for each patch
    for (int i = 0; i < PX_PATCH_BANK_SIZE; i++) {
        if (!px_allocate_patch_memory(&bank->patches[i], config)) {
            // Unwind
            for (int j = 0; j < i; j++) px_free_patch_memory(&bank->patches[j]);
            free(bank->patches);
            free(bank);
            return NULL;
        }
        // Initialize with default values just in case? Or zero is fine.
        // It's calloc'd, so it's zero.
    }

    return bank;
}

PX_API void PX_DestroyPatchBank(PxPatchBank* bank) {
    if (!bank) return;
    if (bank->patches) {
        for (int i = 0; i < PX_PATCH_BANK_SIZE; i++) {
            px_free_patch_memory(&bank->patches[i]);
        }
        free(bank->patches);
    }
    free(bank);
}

PX_API bool PX_Bank_SaveToSlot(PxPatchBank* bank, int slot_idx, PxSynth* s) {
    if (!bank || !s || slot_idx < 0 || slot_idx >= PX_PATCH_BANK_SIZE) return false;

    // Safety Check: Ensure bank configuration is sufficient to hold synth data
    if (bank->config.num_voice_adsrs < s->config.num_voice_adsrs ||
        bank->config.num_lfos < s->config.num_lfos) {
        // fprintf(stderr, "PX_Bank_SaveToSlot: Configuration mismatch (Bank smaller than Synth)\n");
        return false;
    }

    // We use the synth's config for the copy size to avoid copying garbage if bank is larger,
    // but since we checked capacity, it fits.
    // However, deep copy uses the *passed* config for sizes.
    // If we use s->config, we copy only valid data.
    return px_copy_patch_deep(&bank->patches[slot_idx], &s->patch, &s->config);
}

PX_API bool PX_Bank_LoadFromSlot(PxPatchBank* bank, int slot_idx, PxSynth* s) {
    if (!bank || !s || slot_idx < 0 || slot_idx >= PX_PATCH_BANK_SIZE) return false;

    // Safety Check: Ensure synth configuration is sufficient to hold bank data
    // If bank has MORE items than synth, we can only copy what fits, or fail.
    // Failing is safer to prevent data loss or confusion.
    if (s->config.num_voice_adsrs < bank->config.num_voice_adsrs ||
        s->config.num_lfos < bank->config.num_lfos) {
        // fprintf(stderr, "PX_Bank_LoadFromSlot: Configuration mismatch (Synth smaller than Bank)\n");
        return false;
    }

    // Use bank's config to determine how much valid data to copy.
    // Since s has capacity >= bank, this is safe.
    return px_copy_patch_deep(&s->patch, &bank->patches[slot_idx], &bank->config);
}

PX_API bool PX_Bank_CopySlot(PxPatchBank* bank, int src_idx, int dest_idx) {
    if (!bank || src_idx < 0 || src_idx >= PX_PATCH_BANK_SIZE || dest_idx < 0 || dest_idx >= PX_PATCH_BANK_SIZE) return false;
    if (src_idx == dest_idx) return true;
    return px_copy_patch_deep(&bank->patches[dest_idx], &bank->patches[src_idx], &bank->config);
}

#endif // POLYSONIX_PATCHING_IMPLEMENTATION
