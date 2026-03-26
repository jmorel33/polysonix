#define POLYSONIX_IMPLEMENTATION
#define POLYSONIX_PATCHING_IMPLEMENTATION
#include "../polysonix.h"
#include "../px_patching.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Security test for Oscillator and LFO wave_idx bounds checking.
 */

static uint32_t adler32_test(const uint8_t* data, size_t len) {
    uint32_t a = 1, b = 0;
    const uint32_t MOD_ADLER = 65521;
    for (size_t i = 0; i < len; i++) {
        a = (a + data[i]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }
    return (b << 16) | a;
}

typedef struct {
    uint8_t* buffer;
    size_t size;
    size_t pos;
} MemStream;

size_t mem_write(void* token, const void* data, size_t size) {
    MemStream* ms = (MemStream*)token;
    if (ms->pos + size > ms->size) {
        ms->size = ms->pos + size;
        ms->buffer = (uint8_t*)realloc(ms->buffer, ms->size);
    }
    memcpy(ms->buffer + ms->pos, data, size);
    ms->pos += size;
    return size;
}

size_t mem_read(void* token, void* data, size_t size) {
    MemStream* ms = (MemStream*)token;
    if (ms->pos + size > ms->size) size = ms->size - ms->pos;
    memcpy(data, ms->buffer + ms->pos, size);
    ms->pos += size;
    return size;
}

int main() {
    PxConfig config = {
        .num_voices = 1,
        .num_lfos = 1,
        .num_voice_adsrs = 1,
        .sample_rate = 44100.0f,
        .samples_per_lfo_update = 44,
        .osc_update_mode = PX_OSC_UPDATE_MODE_PER_SAMPLE
    };

    PxSynth* synth = PX_Create(&config);
    if (!synth) return 1;

    // 1. Prepare a valid preset
    synth->patch.osc[0].wave_idx = 200;
    synth->patch.osc[0].enabled = true;
    synth->patch.template_lfos[0].wave_idx = 100;
    synth->patch.template_lfos[0].enabled = true;

    MemStream ms = {0};
    PX_SavePresetToBus(synth, mem_write, &ms, "SecurityTest");

    // 2. Corrupt the wave_idx values
    uint32_t data_len = ((uint32_t)ms.buffer[26] << 24) | ((uint32_t)ms.buffer[27] << 16) |
                        ((uint32_t)ms.buffer[28] << 8) | ((uint32_t)ms.buffer[29]);

    // Find Oscillator 0 wave_idx (marker: enabled=1, wave_idx=200)
    // Oscillator serialization uses big-endian write_u32
    uint8_t osc_marker[] = {0x01, 0x00, 0x00, 0x00, 0xC8};
    int osc_pos = -1;
    for (int i = 40; i < (int)(40 + data_len - 5); i++) {
        if (memcmp(ms.buffer + i, osc_marker, 5) == 0) {
            osc_pos = i + 1;
            break;
        }
    }

    // Find LFO 0 wave_idx (marker: wave_idx=100)
    // LFO serialization uses WR_BUF (native endian memcpy)
    // PxLFOParams structure: int wave_idx, float frequency, bool enabled, bool reset_on_key_on, ...
    int lfo_target_val = 100;
    uint8_t lfo_marker[4];
    memcpy(lfo_marker, &lfo_target_val, 4);

    int lfo_pos = -1;
    for (int i = 40; i < (int)(40 + data_len - 4); i++) {
        if (memcmp(ms.buffer + i, lfo_marker, 4) == 0) {
            lfo_pos = i;
            break;
        }
    }

    if (osc_pos == -1 || lfo_pos == -1) {
        printf("FAILED: Could not locate markers (osc:%d lfo:%d)\n", osc_pos, lfo_pos);
        return 1;
    }

    // Corrupt Oscillator wave_idx to 9999 (Big Endian)
    ms.buffer[osc_pos] = 0x00;
    ms.buffer[osc_pos + 1] = 0x00;
    ms.buffer[osc_pos + 2] = 0x27;
    ms.buffer[osc_pos + 3] = 0x0F;

    // Corrupt LFO wave_idx to 8888 (Native Endian)
    int lfo_bad_val = 8888;
    memcpy(ms.buffer + lfo_pos, &lfo_bad_val, 4);

    // Recalculate checksum
    uint32_t new_sum = adler32_test(ms.buffer + 40, data_len);
    ms.buffer[40 + data_len] = (new_sum >> 24) & 0xFF;
    ms.buffer[40 + data_len + 1] = (new_sum >> 16) & 0xFF;
    ms.buffer[40 + data_len + 2] = (new_sum >> 8) & 0xFF;
    ms.buffer[40 + data_len + 3] = new_sum & 0xFF;

    // 3. Load the malicious preset
    ms.pos = 0;
    char error[256];
    if (!PX_LoadPresetFromBus(synth, mem_read, &ms, error, sizeof(error))) {
        printf("FAILED: Load failed: %s\n", error);
        return 1;
    }

    // 4. Verify results
    int loaded_osc_idx = synth->patch.osc[0].wave_idx;
    int loaded_lfo_idx = synth->patch.template_lfos[0].wave_idx;

    bool success = true;
    if (loaded_osc_idx == 0) {
        printf("SUCCESS: osc wave_idx clamped to 0\n");
    } else {
        printf("FAILED: osc wave_idx is %d\n", loaded_osc_idx);
        success = false;
    }

    if (loaded_lfo_idx == 0) {
        printf("SUCCESS: lfo wave_idx clamped to 0\n");
    } else {
        printf("FAILED: lfo wave_idx is %d\n", loaded_lfo_idx);
        success = false;
    }

    // 5. Test API clamping
    float dummy_buf[1024];
    PX_SetOscWave(synth, 0, 7777);
    PX_Process(synth, dummy_buf, 1);
    if (synth->patch.osc[0].wave_idx == 0) {
        printf("SUCCESS: API PX_SetOscWave clamped to 0\n");
    } else {
        printf("FAILED: API PX_SetOscWave allowed %d\n", synth->patch.osc[0].wave_idx);
        success = false;
    }

    PX_SetLFOWaveform(synth, 0, 6666);
    PX_Process(synth, dummy_buf, 1);
    if (synth->patch.template_lfos[0].wave_idx == 0) {
        printf("SUCCESS: API PX_SetLFOWaveform clamped to 0\n");
    } else {
        printf("FAILED: API PX_SetLFOWaveform allowed %d\n", synth->patch.template_lfos[0].wave_idx);
        success = false;
    }

    PX_Destroy(synth);
    free(ms.buffer);
    return success ? 0 : 1;
}
