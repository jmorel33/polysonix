#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <cglm/cglm.h>

#define SITUATION_IMPLEMENTATION
#define SITUATION_USE_OPENGL
#include "situation.h"

#define POLYSONIX_IMPLEMENTATION
#include "../polysonix.h"
#include "polysonix_wave_bank.h"

// --- Global Application State (Not Synth State) ---
static bool enable_drawing = true;

// --- Configuration ---
#define SCREEN_WIDTH        800
#define SCREEN_HEIGHT       750
#define TARGET_FPS          60
#define REQUESTED_SAMPLE_RATE 48000
#define BITS_PER_SAMPLE     16
#define CHANNELS            2
#define SAMPLES_PER_UPDATE  2048 // This will be handled by the audio callback in Situation
#define DRAW_WAVEFORM_HEIGHT 128
#define SINGLE_CYCLE_LENGTH 256 // For waveform drawing


// --- Polysonix Configuration ---
#define NUM_VOICES          8
#define NUM_VOICE_ADSRS     3
#define NUM_LFOS            3
#define LFO_UPDATE_INTERVAL_MS 1.0f

// --- Global Pointers & Buffers ---
static PxSynth* synth = NULL;
static int16_t static_display_buffer[SINGLE_CYCLE_LENGTH];
static SituationSound sound;

// --- UI & Control State ---
static int current_wave_index = 0;
static int octave_shift = 0;
static int last_drawn_wave_index = -1;
static bool last_wave_compile_status = false;


// Keyboard mapping
typedef struct {
    int situation_key;
    int midi_note;
} KeyNoteMapping;

static KeyNoteMapping piano_keys[] = {
    { SIT_KEY_Q, 72 }, { SIT_KEY_2, 73 }, { SIT_KEY_W, 74 }, { SIT_KEY_3, 75 }, { SIT_KEY_E, 76 }, { SIT_KEY_R, 77 }, { SIT_KEY_5, 78 }, { SIT_KEY_T, 79 }, { SIT_KEY_6, 80 }, { SIT_KEY_Y, 81 }, { SIT_KEY_7, 82 }, { SIT_KEY_U, 83 }, { SIT_KEY_I, 84 }, { SIT_KEY_9, 85 },
    { SIT_KEY_O, 86 }, { SIT_KEY_0, 87 }, { SIT_KEY_P, 88 }, { SIT_KEY_LEFT_BRACKET, 89 }, { SIT_KEY_EQUAL, 90 }, { SIT_KEY_RIGHT_BRACKET, 91 }, { SIT_KEY_Z, 60 }, { SIT_KEY_S, 61 }, { SIT_KEY_X, 62 }, { SIT_KEY_D, 63 }, { SIT_KEY_C, 64 }, { SIT_KEY_V, 65 }, { SIT_KEY_G, 66 },
    { SIT_KEY_B, 67 }, { SIT_KEY_H, 68 }, { SIT_KEY_N, 69 }, { SIT_KEY_J, 70 }, { SIT_KEY_M, 71 }, { SIT_KEY_COMMA, 72 }, { SIT_KEY_L, 73 }, { SIT_KEY_PERIOD, 74}, { SIT_KEY_SEMICOLON, 75}, { SIT_KEY_SLASH, 76 }, { SIT_KEY_APOSTROPHE, 77 }, { 0, -1 }
};
#define KEY_OCTAVE_UP   SIT_KEY_RIGHT
#define KEY_OCTAVE_DOWN SIT_KEY_LEFT

// --- Function Prototypes ---
static bool InitializeApplication(int argc, char** argv);
static void CleanupApplication(void);
static void ProcessInput(void);
static void DrawFrame(void);
static bool compile_all_waves();
static void audio_processor_callback(float* buffer, int num_frames, int num_channels, void* user_data);

// --- Application Lifecycle ---

static bool InitializeApplication(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    SituationInitInfo init_info = {
        .app_name = "Polysonix Situation Example",
        .app_version = "1.0",
        .initial_width = SCREEN_WIDTH,
        .initial_height = SCREEN_HEIGHT,
        .window_flags = SITUATION_FLAG_WINDOW_RESIZABLE | SITUATION_FLAG_VSYNC_HINT,
        .target_fps = TARGET_FPS,
        .headless = false
    };

    if (SituationInit(argc, argv, &init_info) != SIT_SUCCESS) {
        printf("Failed to initialize Situation: %s\n", SituationGetLastErrorMsg());
        return false;
    }

    if (!SituationIsAudioDeviceReady()) {
        printf("Audio device not ready.\n");
        return false;
    }

    if (!polysonix_wave_init()) {
        fprintf(stderr, "Failed to initialize Polysonix wave system!\n");
        return false;
    }

    //Create the synthesizer instance
    PxConfig config = {
        .num_voices = NUM_VOICES,
        .num_lfos = NUM_LFOS,
        .num_voice_adsrs = NUM_VOICE_ADSRS,
        .sample_rate = REQUESTED_SAMPLE_RATE,
        .lfo_update_interval_ms = LFO_UPDATE_INTERVAL_MS,
        .samples_per_lfo_update = (int)(REQUESTED_SAMPLE_RATE * (LFO_UPDATE_INTERVAL_MS / 1000.0f)),
        .osc_update_mode = PX_OSC_UPDATE_MODE_PER_SAMPLE,
        .osc_fixed_update_rate_hz = 48000,
        .nyquist_precision_multiplier = 1024.0f
    };
    if (config.samples_per_lfo_update < 1) config.samples_per_lfo_update = 1;

    synth = PX_Create(&config);
    if (!synth) {
        printf("Critical: Failed to create PxSynth instance.\n");
        polysonix_wave_deinit();
        SituationShutdown();
        return false;
    }

    if (!compile_all_waves()) {
        printf("Critical: Wave compilation resulted in zero successful waveforms. Exiting.\n");
        PX_Destroy(synth);
        polysonix_wave_deinit();
        SituationShutdown();
        return false;
    }

    SituationAudioFormat format = {
        .channels = CHANNELS,
        .sample_rate = REQUESTED_SAMPLE_RATE,
        .bit_depth = BITS_PER_SAMPLE
    };
    SituationLoadSoundFromStream(NULL, NULL, NULL, &format, true, &sound);
    SituationAttachAudioProcessor(&sound, audio_processor_callback, NULL);
    SituationPlayLoadedSound(&sound);

    SituationSetTargetFPS(TARGET_FPS);
    return true;
}

static void CleanupApplication() {
    printf("Exiting program.\n");
    SituationUnloadSound(&sound);
    if (synth) PX_Destroy(synth);
    polysonix_wave_print_stats();
    polysonix_wave_deinit();

    for (int i = 0; i < PX_GetNumWaveforms(); ++i) {
        if (default_waves[i].compiled_bytecode != NULL) {
            free_bytecode_chunk(default_waves[i].compiled_bytecode);
            free(default_waves[i].compiled_bytecode);
            default_waves[i].compiled_bytecode = NULL;
        }
    }
    printf("Freed bytecode for %d waveforms.\n", PX_GetNumWaveforms());
    SituationShutdown();
}

static void ProcessInput() {
    if (SituationIsKeyPressed(SIT_KEY_F11)) {
        enable_drawing = !enable_drawing;
        printf("Drawing %s\n", enable_drawing ? "ENABLED" : "DISABLED");
    }

    if (SituationIsKeyPressed(SIT_KEY_DOWN)) current_wave_index = (current_wave_index + 1) % PX_GetNumWaveforms();
    if (SituationIsKeyPressed(SIT_KEY_UP)) current_wave_index = (current_wave_index - 1 + PX_GetNumWaveforms()) % PX_GetNumWaveforms();
    if (SituationIsKeyPressed(KEY_OCTAVE_UP)) { if (octave_shift < 3) octave_shift++; }
    if (SituationIsKeyPressed(KEY_OCTAVE_DOWN)) { if (octave_shift > -3) octave_shift--; }

    // Note On/Off
    for (int i = 0; piano_keys[i].situation_key != 0; ++i) {
        int key = piano_keys[i].situation_key;
        if (SituationIsKeyPressed(key)) {
            int midi_note = piano_keys[i].midi_note + octave_shift * 12;
            midi_note = (int)fmaxf(0.0f, fminf(127.0f, (float)midi_note));
            PX_NoteOn(synth, midi_note, current_wave_index, key);
        }
        if (SituationIsKeyReleased(key)) {
            PX_NoteOff(synth, key);
        }
    }
}

static void DrawFrame() {
    if (SituationAcquireFrameCommandBuffer()) {
        SituationRenderPassInfo pass_info = {
            .color_load_action = SIT_LOAD_ACTION_CLEAR,
            .clear_color = { .r = 245, .g = 245, .b = 245, .a = 255 }, // Ray White
            .color_store_action = SIT_STORE_ACTION_STORE,
        };
        SituationCmdBeginRenderPass(SituationGetMainCommandBuffer(), &pass_info);

        PxWaveInfo waveInfo = PX_GetWaveInfo(current_wave_index);

        if (current_wave_index != last_drawn_wave_index || waveInfo.is_compiled != last_wave_compile_status) {
            if (waveInfo.is_compiled) {
                VmParams display_params = { .rand_offset = 0.5f, .modA = 0.0f, .modB = 0.0f, .modC = 0.0f, .lfsr_type = LFSR_8BIT, .lfsr_state = 1, .lfsr_seed = 1 };
                for (int k = 0; k < SINGLE_CYCLE_LENGTH; ++k) {
                    display_params.x = ((float)k / SINGLE_CYCLE_LENGTH) * 2.0f * PI;
                    float s_f = execute_bytecode(default_waves[current_wave_index].compiled_bytecode, &display_params);
                    static_display_buffer[k] = (int16_t)(fmaxf(-1.0f, fminf(1.0f, s_f)) * 32767.0f);
                }
            }
            last_drawn_wave_index = current_wave_index;
            last_wave_compile_status = waveInfo.is_compiled;
        }

        // --- Static Waveform Display ---
        int waveform_draw_y = 400;
        int wf_x = 10;
        int wf_w = SCREEN_WIDTH - 20;

        if (waveInfo.is_compiled) {
            for (int k = 0; k < SINGLE_CYCLE_LENGTH; ++k) {
                float x = wf_x + (float)k / (SINGLE_CYCLE_LENGTH - 1) * wf_w;
                float y = waveform_draw_y + DRAW_WAVEFORM_HEIGHT / 2.0f - (static_display_buffer[k] / 32768.0f) * (DRAW_WAVEFORM_HEIGHT / 2.0f);

                mat4 transform;
                glm_translate_make(transform, (vec3){x, y, 0.0f});
                glm_scale_uni(transform, 2.0f);

                vec4 color = {1.0f, 0.0f, 0.0f, 1.0f}; // Red
                SituationCmdDrawQuad(SituationGetMainCommandBuffer(), transform, color);
            }
        }

        SituationCmdEndRenderPass(SituationGetMainCommandBuffer());
        SituationEndFrame();
    }
}

static bool compile_all_waves() {
    printf("Compiling %d waveform expressions...\n", PX_GetNumWaveforms());
    int success_count = 0;
    for (int i = 0; i < PX_GetNumWaveforms(); ++i) {
        if (default_waves[i].compiled_bytecode != NULL) {
            free_bytecode_chunk(default_waves[i].compiled_bytecode);
            free(default_waves[i].compiled_bytecode);
            default_waves[i].compiled_bytecode = NULL;
        }
        default_waves[i].compiled_bytecode = compile_expression_to_bytecode(default_waves[i].expression);
        if (default_waves[i].compiled_bytecode != NULL) success_count++;
        else {
            PxWaveInfo info = PX_GetWaveInfo(i);
            fprintf(stderr, "Failed to compile waveform %d ('%s'): %s\n", i, info.name, default_waves[i].expression);
        }
    }
    printf("Finished compiling waveforms (%d successful).\n", success_count);
    return success_count > 0;
}

static void audio_processor_callback(float* buffer, int num_frames, int num_channels, void* user_data) {
    // PX_Process expects a buffer of int16_t, but the callback provides a float buffer.
    // We need to process into a temporary buffer and then convert the samples.
    int16_t temp_buffer[num_frames * num_channels];
    PX_Process(synth, temp_buffer, num_frames);

    // Convert from int16_t to float
    for (int i = 0; i < num_frames * num_channels; i++) {
        buffer[i] = (float)temp_buffer[i] / 32767.0f;
    }
}

int main(int argc, char** argv) {
    if (!InitializeApplication(argc, argv)) {
        printf("Application initialization failed.\n");
        return 1;
    }

    while (!SituationWindowShouldClose()) {
        SituationPollInputEvents();
        SituationUpdateTimers();

        ProcessInput();

        if (enable_drawing) {
            DrawFrame();
        } else {
            if (SituationAcquireFrameCommandBuffer()) {
                SituationRenderPassInfo pass_info = {
                    .color_load_action = SIT_LOAD_ACTION_CLEAR,
                    .clear_color = { .r = 0, .g = 0, .b = 0, .a = 255 },
                    .color_store_action = SIT_STORE_ACTION_STORE,
                };
                SituationCmdBeginRenderPass(SituationGetMainCommandBuffer(), &pass_info);
                SituationCmdEndRenderPass(SituationGetMainCommandBuffer());
                SituationEndFrame();
            }
        }
    }

    CleanupApplication();
    return 0;
}
