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

// "Obfuscated" IO Macros as requested
#define PX_IO_BATCH_WRITE(fn, token, data, size) ((fn)(token, data, size))
#define PX_IO_BATCH_READ(fn, token, data, size)  ((fn)(token, data, size))

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

// Internal macro to handle both size calculation and buffer writing/reading
// ACTION_BUF(pointer, size)
// ACTION_SCALAR(pointer, type)
#define PX_SERIALIZE_BODY(S, ACTION_BUF, ACTION_SCALAR) \
    do { \
        if ((S)->patch.template_voice_adsrs) \
            ACTION_BUF((S)->patch.template_voice_adsrs, sizeof(PxADSRParams) * (S)->config.num_voice_adsrs); \
        if ((S)->patch.template_voice_adsr_mod_amounts) \
            ACTION_BUF((S)->patch.template_voice_adsr_mod_amounts, sizeof(float) * (S)->config.num_voice_adsrs * PX_ADSR_DEST_COUNT); \
        if ((S)->patch.template_lfos) \
            ACTION_BUF((S)->patch.template_lfos, sizeof(PxLFOParams) * (S)->config.num_lfos); \
        ACTION_SCALAR(&(S)->patch.filter_cutoff_hz, float); \
        ACTION_SCALAR(&(S)->patch.filter_resonance_q, float); \
        ACTION_SCALAR(&(S)->patch.filter_env_amount_hz, float); \
        ACTION_SCALAR(&(S)->patch.filter_drive, float); \
        ACTION_SCALAR(&(S)->patch.filter_key_track, float); \
        ACTION_SCALAR(&(S)->patch.filter_poles, int); \
        ACTION_SCALAR(&(S)->patch.filter_mode, PxFilterMode); \
        ACTION_SCALAR(&(S)->patch.voice_pan_setting, float); \
        ACTION_SCALAR(&(S)->patch.default_note_amp, float); \
        ACTION_SCALAR(&(S)->patch.limiter_threshold, float); \
        ACTION_SCALAR(&(S)->patch.limiter_release_ms, float); \
        ACTION_SCALAR(&(S)->patch.unilegato_enabled, bool); \
        ACTION_SCALAR(&(S)->patch.unilegato_slide_duration_s, float); \
        ACTION_BUF((S)->patch.mod_matrix, sizeof(PxModSlot) * PX_MOD_MATRIX_SLOTS); \
        ACTION_SCALAR(&(S)->patch.pitchbend_range_semitones, float); \
        ACTION_SCALAR(&(S)->patch.global_filter_enabled, bool); \
        ACTION_SCALAR(&(S)->patch.global_filter_cutoff_hz, float); \
        ACTION_SCALAR(&(S)->patch.global_filter_resonance_q, float); \
        ACTION_SCALAR(&(S)->patch.global_filter_env_amount_hz, float); \
        ACTION_SCALAR(&(S)->patch.global_filter_drive, float); \
        ACTION_SCALAR(&(S)->patch.global_filter_key_track, float); \
        ACTION_SCALAR(&(S)->patch.global_filter_poles, int); \
        ACTION_SCALAR(&(S)->patch.global_filter_mode, PxFilterMode); \
        ACTION_SCALAR(&(S)->patch.velocity_curve, PxCurveType); \
        ACTION_SCALAR(&(S)->patch.aftertouch_curve, PxCurveType); \
        ACTION_BUF((S)->patch.osc, sizeof(PxOscillator) * PX_MAX_OSC_PER_VOICE); \
    } while(0)

PX_API size_t PX_CalculatePresetSize(PxSynth* s) {
    size_t size = 32; // Header

    #define COUNT_BUF(ptr, sz) size += (sz)
    #define COUNT_SCALAR(ptr, type) size += sizeof(type)

    PX_SERIALIZE_BODY(s, COUNT_BUF, COUNT_SCALAR);

    #undef COUNT_BUF
    #undef COUNT_SCALAR

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

    #define WRITE_BUF(p, sz) do { memcpy(ptr, p, sz); ptr += sz; } while(0)
    #define WRITE_SCALAR(p, type) do { memcpy(ptr, p, sizeof(type)); ptr += sizeof(type); } while(0)

    PX_SERIALIZE_BODY(s, WRITE_BUF, WRITE_SCALAR);

    #undef WRITE_BUF
    #undef WRITE_SCALAR

    // --- Checksum ---
    uint32_t checksum = 0;
    size_t written_data_len = (size_t)(ptr - data_start);
    for (size_t i = 0; i < written_data_len; i++) {
        checksum += data_start[i];
    }

    write_be32_to_buf(ptr, checksum); ptr += 4;

    // --- Batch Write ---
    size_t written = PX_IO_BATCH_WRITE(write_fn, token, buffer, total_size);

    free(buffer);
    return (written == total_size);
}

PX_API bool PX_LoadPresetFromBus(PxSynth* s, PxIOReadFn read_fn, void* token) {
    if (!s || !read_fn) return false;

    // 1. Read Header
    uint8_t header[32];
    if (PX_IO_BATCH_READ(read_fn, token, header, 32) != 32) return false;

    if (strncmp((char*)header, "POLY", 4) != 0) return false;
    // Version check (strict major/minor)
    if (header[4] != POLYSONIX_VERSION_MAJOR || header[5] != POLYSONIX_VERSION_MINOR) return false;

    uint32_t data_len = read_be32_from_buf(header + 24);

    // 2. Allocate Data + Footer
    size_t payload_size = data_len + 4;
    uint8_t* buffer = (uint8_t*)malloc(payload_size);
    if (!buffer) return false;

    // 3. Read Body + Checksum
    if (PX_IO_BATCH_READ(read_fn, token, buffer, payload_size) != payload_size) {
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

    #define READ_BUF(p, sz) do { \
        if (ptr + (sz) > end) { free(buffer); return false; } \
        memcpy(p, ptr, sz); ptr += sz; \
    } while(0)

    #define READ_SCALAR(p, type) do { \
        if (ptr + sizeof(type) > end) { free(buffer); return false; } \
        memcpy(p, ptr, sizeof(type)); ptr += sizeof(type); \
    } while(0)

    PX_SERIALIZE_BODY(s, READ_BUF, READ_SCALAR);

    #undef READ_BUF
    #undef READ_SCALAR

    free(buffer);
    return true;
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

#endif // POLYSONIX_PATCHING_IMPLEMENTATION
