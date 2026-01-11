// --- v1.5 ROM (Populated) ---
// 128 Sequences organized into 16 themed banks of 8.
//
// "Ham Crazy" Edition - Enhanced with complex flag combinations,
// generative probability, and math-based waveforms.

/**
 * =========================================================================================
 *  POLYSONIX WAVE SEQUENCE REFERENCE
 * =========================================================================================
 *
 *  STRUCT: PxWaveSequence (Global Settings)
 *  ----------------------------------------
 *  uint8_t  end_action         : Action when sequence finishes (or PX_WSEQ_END flag hit).
 *                                - PX_WSEQ_END_STOP     (0): Stop voice (Release phase).
 *                                - PX_WSEQ_END_HOLD     (1): Hold last step indefinitely.
 *                                - PX_WSEQ_END_LOOP     (2): Loop back to 'seq_loop_start_idx' (step with LOOP_POINT flag, or 0).
 *                                - PX_WSEQ_END_PINGPONG (3): Reverse playback direction.
 *                                - PX_WSEQ_END_REVERSE  (4): Play backwards.
 *
 *  uint8_t  glide_mode         : Pitch transition style.
 *                                - PX_WSEQ_GLIDE_OFF    (0): Stepped pitch changes.
 *                                - PX_WSEQ_GLIDE_STEP   (1): Linear glide within step duration.
 *                                - PX_WSEQ_GLIDE_SMOOTH (2): Continuous RC filter glide (ignores step bounds).
 *
 *  uint8_t  bitcrush_bits      : Base bit depth for PX_WSEQ_BITCRUSH (1-16, typically 4-8).
 *  uint8_t  adsr_retrig_phase  : Target ADSR state on PX_WSEQ_RETRIG_ADSR (1=Attack, 2=Decay, etc).
 *
 *  uint8_t  prob_mute_score    : % Chance (0-100) to mute step if PX_WSEQ_USE_PROB_MUTE is set.
 *  uint8_t  prob_skip_score    : % Chance (0-100) to skip step if PX_WSEQ_USE_PROB_SKIP is set.
 *  uint8_t  rnd_octave_range   : % Chance (0-100) to shift +/- 1 octave if PX_WSEQ_USE_RND_OCTAVE is set.
 *  uint8_t  reset_lfo_pos      : Boolean (1=Yes) to reset LFOs on sequence start.
 *
 *  uint16_t rnd_wave_low       : Start index for random wave range (PX_WSEQ_USE_RND_WAVE).
 *  uint16_t rnd_wave_high      : End index for random wave range.
 *
 *  int8_t   [x]_mod_src        : Mod Source (-1 = None/Internal, 0 = Velocity, 1 = ModWheel, etc).
 *  float    [x]_depth          : Base effect depth (0.0 - 1.0).
 *
 *
 *  STRUCT: PxWaveSeqStep (Per-Step Data)
 *  -------------------------------------
 *  uint16_t wave_idx           : Waveform Index (0-255+).
 *  uint16_t duration_cycles    : Duration in oscillator cycles (e.g., 100).
 *  int16_t  pitch_offset       : Tuning in cents (+/-). 1200 = 1 Octave.
 *  uint16_t flags              : Bitwise logic flags.
 *
 *
 *  BIT FLAGS (16-bit)
 *  ------------------
 *  [Flow Control]
 *  (1<<0) PX_WSEQ_END             : Force sequence end action here.
 *  (1<<1) PX_WSEQ_LOOP_POINT      : Mark this step as the start point for LOOP mode.
 *  (1<<2) PX_WSEQ_JUMP_RANDOM     : Jump to a random step index (0-63).
 *
 *  [Reset/Mod]
 *  (1<<4) PX_WSEQ_RESET_LFO       : Reset LFO phase to 0.
 *  (1<<5) PX_WSEQ_RETRIG_ADSR     : Retrigger Voice ADSRs.
 *  (1<<6) PX_WSEQ_LOCK_PHASE      : Hard Sync oscillator phase to 0.
 *  (1<<7) PX_WSEQ_GLIDE           : Enable per-step exponential glide (overrides Linear).
 *
 *  [Generative]
 *  (1<<8) PX_WSEQ_USE_PROB_MUTE   : Randomly mute this step (uses prob_mute_score).
 *  (1<<9) PX_WSEQ_USE_PROB_SKIP   : Randomly skip this step (zero duration) (uses prob_skip_score).
 *  (1<<10) PX_WSEQ_USE_RND_OCTAVE : Randomly offset pitch +/- 1200 cents (uses rnd_octave_range).
 *  (1<<11) PX_WSEQ_USE_RND_WAVE   : Randomly pick wave_idx from [low, high] range.
 *
 *  [Timbre/FX]
 *  (1<<12) PX_WSEQ_REVERSE_PLAY   : Play waveform backwards (negative freq).
 *  (1<<13) PX_WSEQ_BITCRUSH       : Enable Bitcrush effect.
 *  (1<<14) PX_WSEQ_XMOD           : Enable Cross-Mod (FM from prev Osc).
 *  (1<<15) PX_WSEQ_RING_MOD       : Enable Ring Mod (AM from prev Osc).
 *
 * =========================================================================================
 */

static const PxWaveSequence ROM_WAVE_SEQUENCES[PX_NUM_WSEQ_BANKS] = {

    // --- Bank 0: Lead (0-7) ---
    // Leads are focused on melodic playback, often with glide or expressive articulation.
    // 0: Classic Saw Lead (Glide enabled, slight detune feeling via sequence)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 6, .duration_cycles = 100, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE}, // Saw Rising
            {.wave_idx = 7, .duration_cycles = 100, .pitch_offset = 5, .flags = PX_WSEQ_GLIDE}, // Saw Falling (detune)
            {.wave_idx = 6, .duration_cycles = 100, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 7, .duration_cycles = 100, .pitch_offset = -5, .flags = PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 1: Pulse Width Modulation (Simulated by switching Pulse waves)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 32, .duration_cycles = 20, .pitch_offset = 0, .flags = 0}, // Pulse 25%
            {.wave_idx = 4,  .duration_cycles = 20, .pitch_offset = 0, .flags = 0}, // Square
            {.wave_idx = 33, .duration_cycles = 20, .pitch_offset = 0, .flags = 0}, // Pulse 75%
            {.wave_idx = 4,  .duration_cycles = 20, .pitch_offset = 0, .flags = 0}, // Square
            {.flags = PX_WSEQ_END}
        }
    },
    // 2: FM Solo (Expressive with XMod)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .glide_mode = PX_WSEQ_GLIDE_STEP,
        .xmod_depth = 0.3f,
        .xmod_mod_src = -1,
        .steps = {
            {.wave_idx = 70, .duration_cycles = 50, .pitch_offset = 0, .flags = PX_WSEQ_XMOD}, // FM Dynamic Lead
            {.wave_idx = 70, .duration_cycles = 50, .pitch_offset = 10, .flags = 0}, // Vibrato-ish
            {.wave_idx = 70, .duration_cycles = 50, .pitch_offset = 0, .flags = PX_WSEQ_XMOD},
            {.wave_idx = 70, .duration_cycles = 50, .pitch_offset = -10, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 3: Sync Lead (Hard Sync effect)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 116, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_LOCK_PHASE}, // Sync Sweep
            {.wave_idx = 117, .duration_cycles = 200, .pitch_offset = 0, .flags = 0},
            {.flags = PX_WSEQ_END}
        },
        .lock_phase_mod_src = -1
    },
    // 4: Bitcrushed Lead
    {
        .end_action = PX_WSEQ_END_LOOP,
        .bitcrush_bits = 4,
        .steps = {
            {.wave_idx = 2, .duration_cycles = 100, .pitch_offset = 0, .flags = PX_WSEQ_BITCRUSH}, // Sine
            {.wave_idx = 6, .duration_cycles = 100, .pitch_offset = 0, .flags = PX_WSEQ_BITCRUSH}, // Saw
            {.flags = PX_WSEQ_END}
        }
    },
    // 5: Glitch Arp (Generative Octave Jumps with Bitcrush & Ring Mod)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .rnd_octave_range = 40,
        .bitcrush_bits = 6,
        .ring_mod_depth = 0.3f,
        .ring_mod_mod_src = -1,
        .steps = {
            {.wave_idx = 4, .duration_cycles = 150, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE | PX_WSEQ_RING_MOD},
            {.wave_idx = 4, .duration_cycles = 150, .pitch_offset = 400, .flags = PX_WSEQ_USE_RND_OCTAVE | PX_WSEQ_BITCRUSH | PX_WSEQ_GLIDE}, // Stuttering Morph
            {.wave_idx = 4, .duration_cycles = 150, .pitch_offset = 700, .flags = PX_WSEQ_RING_MOD},
            {.flags = PX_WSEQ_END}
        }
    },
    // 6: Phase Dist Lead
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 56, .duration_cycles = 300, .pitch_offset = 0, .flags = 0}, // Phase Distortion
            {.wave_idx = 60, .duration_cycles = 300, .pitch_offset = 0, .flags = 0}, // PD Resonant
            {.flags = PX_WSEQ_END}
        }
    },
    // 7: Resonant Lead
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 114, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, // Reso Filter Sweep
            {.flags = PX_WSEQ_END}
        }
    },

    // --- Bank 1: Pad (8-15) ---
    // Pads are slow-attack, evolving textures suitable for chords and atmosphere.
    // 8: PWM Pad (Slow evolution)
    {
        .end_action = PX_WSEQ_END_PINGPONG,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 32, .duration_cycles = 1000, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 4,  .duration_cycles = 1000, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 33, .duration_cycles = 1000, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 9: Generative Ambient (Replaced Glassy Pad)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .rnd_wave_low = 80,
        .rnd_wave_high = 100,
        .prob_mute_score = 50,
        .steps = {
            {.wave_idx = 0, .duration_cycles = 800, .pitch_offset = 0, .flags = PX_WSEQ_USE_RND_WAVE | PX_WSEQ_USE_PROB_MUTE}, // Evolving texture
            {.wave_idx = 0, .duration_cycles = 800, .pitch_offset = 0, .flags = PX_WSEQ_USE_RND_WAVE | PX_WSEQ_REVERSE_PLAY},
            {.flags = PX_WSEQ_END}
        }
    },
    // 10: Choir Pad
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 121, .duration_cycles = 800, .pitch_offset = 0, .flags = 0}, // Oooh Choir
            {.wave_idx = 113, .duration_cycles = 800, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE}, // Vocal Ah
            {.flags = PX_WSEQ_END}
        }
    },
    // 11: Ring Mod Morph (Updated)
    {
        .end_action = PX_WSEQ_END_PINGPONG,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .prob_skip_score = 30,
        .steps = {
            {.wave_idx = 107, .duration_cycles = 600, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 108, .duration_cycles = 600, .pitch_offset = 5, .flags = PX_WSEQ_GLIDE | PX_WSEQ_USE_PROB_SKIP}, // Chance to skip detune
            {.wave_idx = 109, .duration_cycles = 600, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE | PX_WSEQ_RING_MOD}, // Added Ring Mod + Glide
            {.flags = PX_WSEQ_END}
        },
        .ring_mod_depth = 0.2f,
        .ring_mod_mod_src = -1
    },
    // 12: Self-Xmod Drone
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 81, .duration_cycles = 2000, .pitch_offset = -1200, .flags = PX_WSEQ_XMOD}, // FM Hollow Drone with Self-FM
            {.flags = PX_WSEQ_END}
        },
        .xmod_depth = 0.5f,
        .xmod_mod_src = -1
    },
    // 13: Space Pad
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 79, .duration_cycles = 1500, .pitch_offset = 0, .flags = 0}, // FM Sci-Fi Drone
            {.flags = PX_WSEQ_END}
        }
    },
    // 14: Shimmer Pad
    {
        .end_action = PX_WSEQ_END_PINGPONG,
        .steps = {
            {.wave_idx = 102, .duration_cycles = 400, .pitch_offset = 0, .flags = 0}, // Classic Pad
            {.wave_idx = 102, .duration_cycles = 400, .pitch_offset = 1200, .flags = PX_WSEQ_GLIDE}, // +1 Octave shimmer
            {.flags = PX_WSEQ_END}
        }
    },
    // 15: Vintage Strings
    {
        .end_action = PX_WSEQ_END_LOOP,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 8, .duration_cycles = 300, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE}, // Saw/Sine
            {.wave_idx = 9, .duration_cycles = 300, .pitch_offset = 5, .flags = PX_WSEQ_GLIDE}, // Beating Detune (+5 cents)
            {.flags = PX_WSEQ_END}
        }
    },

    // --- Bank 2: Strings (16-23) ---
    // Orchestral and synthesized string emulations.
    // 16: Bowed String
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 99, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, // Bowed String
            {.flags = PX_WSEQ_END}
        }
    },
    // 17: String Ensemble
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 147, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, // Rich String Ensemble
            {.flags = PX_WSEQ_END}
        }
    },
    // 18: Tremolo Strings
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 147, .duration_cycles = 1500, .pitch_offset = 0, .flags = PX_WSEQ_RING_MOD}, // Use Ring Mod as Tremolo
            {.flags = PX_WSEQ_END}
        },
        .ring_mod_depth = 0.3f,
        .ring_mod_mod_src = -1
    },
    // 19: Pizzicato
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = {
            {.wave_idx = 144, .duration_cycles = 200, .pitch_offset = 0, .flags = 0}, // Plucked String
            {.flags = PX_WSEQ_END}
        }
    },
    // 20: Slow Strings
    {
        .end_action = PX_WSEQ_END_LOOP,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 147, .duration_cycles = 500, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 147, .duration_cycles = 500, .pitch_offset = 5, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 147, .duration_cycles = 500, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 147, .duration_cycles = 500, .pitch_offset = -5, .flags = PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 21: Octave Strings
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 147, .duration_cycles = 300, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 147, .duration_cycles = 300, .pitch_offset = 1200, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 22: Crushed Bow (Updated from Crushed Strings)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .bitcrush_bits = 6,
        .prob_skip_score = 40,
        .steps = {
            {.wave_idx = 147, .duration_cycles = 200, .pitch_offset = -5, .flags = PX_WSEQ_BITCRUSH | PX_WSEQ_USE_PROB_SKIP}, // Skip creates stutter
            {.wave_idx = 147, .duration_cycles = 200, .pitch_offset = 5, .flags = PX_WSEQ_BITCRUSH},
            {.flags = PX_WSEQ_END}
        }
    },
    // 23: Reverse Swell Strings (Replaced Evolving Strings)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 147, .duration_cycles = 800, .pitch_offset = 0, .flags = PX_WSEQ_REVERSE_PLAY | PX_WSEQ_GLIDE},
            {.wave_idx = 147, .duration_cycles = 800, .pitch_offset = 1200, .flags = PX_WSEQ_REVERSE_PLAY | PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },

    // --- Bank 3: Choir (24-31) ---
    // Vocal-like formants and choir textures.
    // 24: Ooh Choir
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 121, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 25: Aah Choir
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 113, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 26: Vowel Morph
    {
        .end_action = PX_WSEQ_END_PINGPONG,
        .steps = {
            {.wave_idx = 122, .duration_cycles = 600, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 115, .duration_cycles = 600, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 27: Robot Voice
    {
        .end_action = PX_WSEQ_END_LOOP,
        .bitcrush_bits = 6,
        .steps = {
            {.wave_idx = 112, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_BITCRUSH}, // Formantish
            {.flags = PX_WSEQ_END}
        }
    },
    // 28: Alien Choir
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 218, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} } // Alien Voice
    },
    // 29: Whispers
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 113, .duration_cycles = 100, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 141, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, // Filtered Static
            {.flags = PX_WSEQ_END}
        }
    },
    // 30: Angelic
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 121, .duration_cycles = 200, .pitch_offset = 1200, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 31: Monks
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 113, .duration_cycles = 200, .pitch_offset = -1200, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },

    // --- Bank 4: Ensemble (32-39) ---
    // Brass, Wind, and full orchestra hits.
    // 32: Brass Section
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 148, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 33: Synth Brass
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 98, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 34: Orchestra Hit
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = {
            {.wave_idx = 148, .duration_cycles = 20, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 148, .duration_cycles = 20, .pitch_offset = 1200, .flags = 0}, // Octave stab
            {.wave_idx = 148, .duration_cycles = 20, .pitch_offset = 0, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 35: Wind Section
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 125, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} } // Breathy Flute
    },
    // 36: Fanfare
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = {
            {.wave_idx = 98, .duration_cycles = 100, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 98, .duration_cycles = 100, .pitch_offset = 400, .flags = 0}, // Major 3rd
            {.wave_idx = 98, .duration_cycles = 100, .pitch_offset = 700, .flags = 0}, // 5th
            {.wave_idx = 98, .duration_cycles = 400, .pitch_offset = 1200, .flags = 0}, // Octave
            {.flags = PX_WSEQ_END}
        }
    },
    // 37: Big Band
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 148, .duration_cycles = 100, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 6, .duration_cycles = 100, .pitch_offset = -1200, .flags = 0}, // Low saw
            {.flags = PX_WSEQ_END}
        }
    },
    // 38: Epic Hit
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = {
            {.wave_idx = 128, .duration_cycles = 10, .pitch_offset = -2400, .flags = 0}, // Kick
            {.wave_idx = 148, .duration_cycles = 200, .pitch_offset = 0, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 39: Detuned Saw Stack
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 6, .duration_cycles = 50, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 7, .duration_cycles = 50, .pitch_offset = 15, .flags = 0},
            {.wave_idx = 6, .duration_cycles = 50, .pitch_offset = -15, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },

    // --- Bank 5: Pluck (40-47) ---
    // Short, percussive tonal sounds mimicking plucked instruments.
    // 40: Nylon Guitar
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 144, .duration_cycles = 300, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 41: Harp Arp
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 144, .duration_cycles = 200, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 144, .duration_cycles = 200, .pitch_offset = 400, .flags = 0},
            {.wave_idx = 144, .duration_cycles = 200, .pitch_offset = 700, .flags = 0},
            {.wave_idx = 144, .duration_cycles = 200, .pitch_offset = 1200, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 42: Koto
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = {
            {.wave_idx = 68, .duration_cycles = 10, .pitch_offset = 200, .flags = 0}, // FM Pluck bend
            {.wave_idx = 68, .duration_cycles = 200, .pitch_offset = 0, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 43: Banjo
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 6, .duration_cycles = 150, .pitch_offset = 0, .flags = PX_WSEQ_BITCRUSH}, {.flags = PX_WSEQ_END} },
        .bitcrush_bits = 7
    },
    // 44: Muted Guitar
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 144, .duration_cycles = 50, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} } // Short duration
    },
    // 45: Electric Pluck
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 6, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_BITCRUSH}, {.flags = PX_WSEQ_END} },
        .bitcrush_bits = 4
    },
    // 46: Bass Pluck
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 144, .duration_cycles = 300, .pitch_offset = -1200, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 47: Random Pluck
    {
        .end_action = PX_WSEQ_END_LOOP,
        .rnd_octave_range = 50,
        .steps = {
            {.wave_idx = 144, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_USE_RND_OCTAVE},
            {.flags = PX_WSEQ_END}
        }
    },

    // --- Bank 6: Percussive (48-55) ---
    // Drums and hits.
    // 48: Kick
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 128, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 49: Snare
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 129, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 50: Math Hat (Replaced HiHat)
    {
        .end_action = PX_WSEQ_END_STOP,
        .bitcrush_bits = 3,
        .prob_skip_score = 30,
        .steps = {
            {.wave_idx = 242, .duration_cycles = 20, .pitch_offset = 0, .flags = PX_WSEQ_BITCRUSH | PX_WSEQ_USE_PROB_SKIP}, // Digital Saw
            {.wave_idx = 245, .duration_cycles = 30, .pitch_offset = 0, .flags = PX_WSEQ_BITCRUSH}, // White Noise
            {.flags = PX_WSEQ_END}
        }
    },
    // 51: Tom
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 131, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 52: Evolving Glitch Perc (Updated)
    {
        .end_action = PX_WSEQ_END_STOP,
        .bitcrush_bits = 5,
        .prob_skip_score = 40,
        .rnd_octave_range = 30,
        .steps = {
            {.wave_idx = 143, .duration_cycles = 100, .pitch_offset = 0, .flags = PX_WSEQ_RESET_LFO | PX_WSEQ_RETRIG_ADSR | PX_WSEQ_USE_PROB_SKIP | PX_WSEQ_BITCRUSH | PX_WSEQ_USE_RND_OCTAVE}, // Max flags
            {.flags = PX_WSEQ_END}
        },
        .adsr_retrig_phase = 1 // Retrigger Attack
    },
    // 53: Industrial Hit
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 136, .duration_cycles = 150, .pitch_offset = -1200, .flags = 0}, {.flags = PX_WSEQ_END} } // Metallic Perc low
    },
    // 54: Zap
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 211, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} } // Laser Zap
    },
    // 55: Clave
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 142, .duration_cycles = 50, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} } // Wooden Perc
    },

    // --- Bank 7: Oldskool (56-63) ---
    // Retro game and chiptune sounds.
    // 56: Basic Arp
    {
        .end_action = PX_WSEQ_END_PINGPONG,
        .steps = {
            {.wave_idx = 4, .duration_cycles = 100, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 4, .duration_cycles = 100, .pitch_offset = 1200, .flags = 0},
            {.wave_idx = 4, .duration_cycles = 100, .pitch_offset = 2400, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 57: 8-bit Run
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 5, .duration_cycles = 50, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 5, .duration_cycles = 50, .pitch_offset = 200, .flags = 0},
            {.wave_idx = 5, .duration_cycles = 50, .pitch_offset = 400, .flags = 0},
            {.wave_idx = 5, .duration_cycles = 50, .pitch_offset = 500, .flags = 0},
            {.wave_idx = 5, .duration_cycles = 50, .pitch_offset = 700, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 58: C64 Arp
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 4, .duration_cycles = 20, .pitch_offset = 0, .flags = 0}, // Very fast
            {.wave_idx = 4, .duration_cycles = 20, .pitch_offset = 300, .flags = 0},
            {.wave_idx = 4, .duration_cycles = 20, .pitch_offset = 700, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 59: Mario Jump
    {
        .end_action = PX_WSEQ_END_STOP,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 4, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 4, .duration_cycles = 200, .pitch_offset = 1200, .flags = PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 60: Coin
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = {
            {.wave_idx = 32, .duration_cycles = 50, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 32, .duration_cycles = 200, .pitch_offset = 500, .flags = 0}, // 4th/5th up
            {.flags = PX_WSEQ_END}
        }
    },
    // 61: Power Up
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = {
            {.wave_idx = 33, .duration_cycles = 100, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 33, .duration_cycles = 100, .pitch_offset = 400, .flags = 0},
            {.wave_idx = 33, .duration_cycles = 100, .pitch_offset = 700, .flags = 0},
            {.wave_idx = 33, .duration_cycles = 100, .pitch_offset = 1200, .flags = 0},
            {.wave_idx = 33, .duration_cycles = 100, .pitch_offset = 1600, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 62: Game Over
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = {
            {.wave_idx = 6, .duration_cycles = 300, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 6, .duration_cycles = 300, .pitch_offset = -100, .flags = 0},
            {.wave_idx = 6, .duration_cycles = 400, .pitch_offset = -200, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 63: Chiptune Lead
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 32, .duration_cycles = 200, .pitch_offset = 0, .flags = 0}, // Pulse 25
            {.wave_idx = 34, .duration_cycles = 200, .pitch_offset = 0, .flags = 0}, // Staircase
            {.flags = PX_WSEQ_END}
        }
    },

    // --- Bank 8: Arcade (64-71) ---
    // Classic arcade SFX and aggressive digital sounds.
    // 64: Pac-Man
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 176, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 65: Invader
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 181, .duration_cycles = 200, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 66: Explosion
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 183, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 67: Laser
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 190, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 68: Jump
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 197, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 69: Collect
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 215, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 70: Enemy
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 218, .duration_cycles = 300, .pitch_offset = -1200, .flags = PX_WSEQ_XMOD}, // Added growl
            {.flags = PX_WSEQ_END}
        },
        .xmod_depth = 0.3f,
        .xmod_mod_src = -1
    },
    // 71: Level Up
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 219, .duration_cycles = 200, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },

    // --- Bank 9: Fun (72-79) ---
    // Novelty and cartoon effects.
    // 72: Bubble
    {
        .end_action = PX_WSEQ_END_STOP,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 2, .duration_cycles = 100, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 2, .duration_cycles = 100, .pitch_offset = 1200, .flags = PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 73: Squeak
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 2, .duration_cycles = 50, .pitch_offset = 2400, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 74: Wobble
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 2, .duration_cycles = 100, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 2, .duration_cycles = 100, .pitch_offset = 200, .flags = 0},
            {.wave_idx = 2, .duration_cycles = 100, .pitch_offset = -200, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 75: Boing
    {
        .end_action = PX_WSEQ_END_STOP,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 207, .duration_cycles = 200, .pitch_offset = 0, .flags = 0}, // Moon Patrol Bounce
            {.flags = PX_WSEQ_END}
        }
    },
    // 76: Slide Whistle
    {
        .end_action = PX_WSEQ_END_PINGPONG,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 2, .duration_cycles = 500, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 2, .duration_cycles = 500, .pitch_offset = 1200, .flags = PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 77: Cartoon Fall
    {
        .end_action = PX_WSEQ_END_STOP,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 2, .duration_cycles = 500, .pitch_offset = 2400, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 2, .duration_cycles = 500, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 78: Toy Piano
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 217, .duration_cycles = 200, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 79: Kazoo
    {
        .end_action = PX_WSEQ_END_LOOP,
        .bitcrush_bits = 3,
        .steps = {
            {.wave_idx = 6, .duration_cycles = 100, .pitch_offset = 0, .flags = PX_WSEQ_BITCRUSH},
            {.flags = PX_WSEQ_END}
        }
    },

    // --- Bank 10: Natural (80-87) ---
    // Environmental and organic simulations.
    // 80: Wind
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 159, .duration_cycles = 500, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} } // Wind AM
    },
    // 81: Rain
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 156, .duration_cycles = 50, .pitch_offset = 0, .flags = PX_WSEQ_USE_PROB_SKIP}, {.flags = PX_WSEQ_END} }, // Water Droplet
        .prob_skip_score = 50
    },
    // 82: Thunder
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 140, .duration_cycles = 500, .pitch_offset = -2400, .flags = 0}, {.flags = PX_WSEQ_END} } // Rumble Noise
    },
    // 83: Bird
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 55, .duration_cycles = 200, .pitch_offset = 2400, .flags = 0}, {.flags = PX_WSEQ_END} } // Bird Call AM
    },
    // 84: Insect
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 6, .duration_cycles = 10, .pitch_offset = 3600, .flags = 0}, {.flags = PX_WSEQ_END} } // High saw
    },
    // 85: Water
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 156, .duration_cycles = 200, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 86: Fire
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 143, .duration_cycles = 50, .pitch_offset = 0, .flags = PX_WSEQ_USE_RND_OCTAVE}, {.flags = PX_WSEQ_END} },
        .rnd_octave_range = 80
    },
    // 87: Ocean
    {
        .end_action = PX_WSEQ_END_PINGPONG,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 245, .duration_cycles = 2000, .pitch_offset = -1200, .flags = PX_WSEQ_GLIDE}, // White noise
            {.wave_idx = 245, .duration_cycles = 2000, .pitch_offset = -2400, .flags = PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },

    // --- Bank 11: Enhanced (88-95) ---
    // Modern complex patches using advanced features.
    // 88: Super Saw
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 6, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 89: Hyper Square
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 4, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 90: Trance Gate
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 6, .duration_cycles = 50, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 6, .duration_cycles = 50, .pitch_offset = 0, .flags = PX_WSEQ_USE_PROB_MUTE}, // Gate effect
            {.flags = PX_WSEQ_END}
        },
        .prob_mute_score = 100 // Always mute step 2
    },
    // 91: Complex Arp
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 6, .duration_cycles = 50, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 6, .duration_cycles = 50, .pitch_offset = 1200, .flags = 0},
            {.wave_idx = 6, .duration_cycles = 50, .pitch_offset = 700, .flags = 0},
            {.wave_idx = 6, .duration_cycles = 50, .pitch_offset = 1900, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 92: Hyper Stutter (Replaced Morphing Lead)
    {
        .end_action = PX_WSEQ_END_PINGPONG,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .bitcrush_bits = 5,
        .prob_skip_score = 25,
        .steps = {
            {.wave_idx = 6, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE | PX_WSEQ_BITCRUSH},
            {.wave_idx = 4, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE | PX_WSEQ_USE_PROB_SKIP},
            {.flags = PX_WSEQ_END}
        }
    },
    // 93: Generative Glitch (Replaced Stutter)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .prob_mute_score = 30,
        .rnd_wave_low = 0,
        .rnd_wave_high = 64,
        .rnd_octave_range = 60,
        .steps = {
            {.wave_idx = 6, .duration_cycles = 20, .pitch_offset = 0, .flags = PX_WSEQ_USE_RND_WAVE | PX_WSEQ_USE_RND_OCTAVE}, // Random wave & octave
            {.wave_idx = 6, .duration_cycles = 20, .pitch_offset = 0, .flags = PX_WSEQ_USE_PROB_MUTE | PX_WSEQ_RETRIG_ADSR}, // Mute & Retrigger
            {.flags = PX_WSEQ_END}
        },
        .adsr_retrig_phase = 1
    },
    // 94: Glitch Hop
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 143, .duration_cycles = 50, .pitch_offset = 0, .flags = PX_WSEQ_BITCRUSH | PX_WSEQ_RING_MOD}, // Added Ring Mod
            {.wave_idx = 143, .duration_cycles = 50, .pitch_offset = 1200, .flags = PX_WSEQ_BITCRUSH},
            {.flags = PX_WSEQ_END}
        },
        .bitcrush_bits = 4,
        .ring_mod_depth = 0.2f,
        .ring_mod_mod_src = -1
    },
    // 95: Total Chaos Theory
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 248, .duration_cycles = 50, .pitch_offset = 0, .flags = PX_WSEQ_USE_RND_OCTAVE | PX_WSEQ_USE_RND_WAVE | PX_WSEQ_REVERSE_PLAY | PX_WSEQ_RING_MOD}, // Logistic Chaos + Flag Overflow
            {.flags = PX_WSEQ_END}
        },
        .rnd_octave_range = 75,
        .rnd_wave_low = 0,
        .rnd_wave_high = 255,
        .ring_mod_depth = 0.4f,
        .ring_mod_mod_src = -1
    },

    // --- Bank 12: Deep (96-103) ---
    // Sub-bass and heavy low-end textures.
    // 96: Sub Bass
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 74, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} } // FM Deep Sub
    },
    // 97: Dub Chord
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 106, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} } // Minor Triad
    },
    // 98: FM Math Drone (Replaced Chaos Drone)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .xmod_depth = 0.6f,
        .xmod_mod_src = -1,
        .steps = {
            {.wave_idx = 50, .duration_cycles = 1000, .pitch_offset = -1200, .flags = PX_WSEQ_XMOD | PX_WSEQ_LOCK_PHASE}, // Laser Malfunction with XMod & Phase Lock
            {.flags = PX_WSEQ_END}
        },
        .lock_phase_mod_src = -1 // Always lock
    },
    // 99: 808 Kick
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 128, .duration_cycles = 300, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 100: Dark Ambient
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 153, .duration_cycles = 800, .pitch_offset = -1200, .flags = 0}, {.flags = PX_WSEQ_END} } // Chaotic Osc
    },
    // 101: Underwater
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 156, .duration_cycles = 500, .pitch_offset = -1200, .flags = 0}, {.flags = PX_WSEQ_END} } // Water Droplet
    },
    // 102: Heartbeat
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = {
            {.wave_idx = 128, .duration_cycles = 50, .pitch_offset = -2400, .flags = 0},
            {.wave_idx = 128, .duration_cycles = 50, .pitch_offset = -2400, .flags = PX_WSEQ_USE_PROB_MUTE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 103: Rumble
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 140, .duration_cycles = 500, .pitch_offset = -2400, .flags = 0}, {.flags = PX_WSEQ_END} }
    },

    // --- Bank 13: Futuristic (104-111) ---
    // Sci-fi, cyber, and technological sounds.
    // 104: Robot Talk
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 157, .duration_cycles = 50, .pitch_offset = 0, .flags = PX_WSEQ_USE_RND_OCTAVE}, // Alien Chatter
            {.flags = PX_WSEQ_END}
        },
        .rnd_octave_range = 30
    },
    // 105: Data Stream
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 160, .duration_cycles = 10, .pitch_offset = 0, .flags = PX_WSEQ_USE_RND_OCTAVE}, // LFSR Rhythm Gate
            {.flags = PX_WSEQ_END}
        },
        .rnd_octave_range = 80
    },
    // 106: Cyberpunk Bass
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 210, .duration_cycles = 100, .pitch_offset = -1200, .flags = 0}, {.flags = PX_WSEQ_END} } // POKEY Distorted
    },
    // 107: Math Laser (Replaced Laser Harp)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .xmod_depth = 0.4f,
        .xmod_mod_src = -1,
        .steps = {
            {.wave_idx = 50, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_XMOD | PX_WSEQ_GLIDE}, // Laser Malfunction
            {.wave_idx = 50, .duration_cycles = 200, .pitch_offset = 1200, .flags = PX_WSEQ_XMOD | PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 108: Teleport
    {
        .end_action = PX_WSEQ_END_STOP,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 48, .duration_cycles = 300, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE}, // Warp Speed
            {.wave_idx = 48, .duration_cycles = 300, .pitch_offset = 2400, .flags = PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 109: Scanner
    {
        .end_action = PX_WSEQ_END_PINGPONG,
        .steps = {
            {.wave_idx = 221, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, // Sweep Down
            {.wave_idx = 221, .duration_cycles = 100, .pitch_offset = 500, .flags = 0},
            {.flags = PX_WSEQ_END}
        }
    },
    // 110: Matrix
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 175, .duration_cycles = 50, .pitch_offset = 0, .flags = 0}, // LFSR Glitch Matrix
            {.flags = PX_WSEQ_END}
        }
    },
    // 111: Warp Drive
    {
        .end_action = PX_WSEQ_END_STOP,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 48, .duration_cycles = 1000, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 48, .duration_cycles = 1000, .pitch_offset = 3600, .flags = PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },

    // --- Bank 14: Emulation (112-119) ---
    // Approximations of acoustic instruments using simple waveforms.
    // 112: Organ
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 110, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 113: Flute
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 125, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 114: Clarinet
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 4, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} } // Square
    },
    // 115: Trumpet
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 98, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 116: Violin
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 99, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 117: Cello
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 99, .duration_cycles = 100, .pitch_offset = -1200, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 118: Bell
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 88, .duration_cycles = 400, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} } // Metallic Bell
    },
    // 119: Steel Drum
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 136, .duration_cycles = 200, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },

    // --- Bank 15: Strange (120-127) ---
    // Generative, chaotic, and experimental "Ham Crazy" sequences.
    // 120: Fractal Spiral (Replaced Fibonacci Spiral)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .ring_mod_depth = 0.3f,
        .ring_mod_mod_src = -1,
        .xmod_depth = 0.3f,
        .xmod_mod_src = -1,
        .steps = {
            {.wave_idx = 247, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE | PX_WSEQ_RING_MOD | PX_WSEQ_XMOD}, // Fibonacci with Ring+XMod
            {.wave_idx = 247, .duration_cycles = 200, .pitch_offset = 700, .flags = PX_WSEQ_GLIDE | PX_WSEQ_RING_MOD | PX_WSEQ_XMOD},
            {.flags = PX_WSEQ_END}
        }
    },
    // 121: Chaos Theory (Replaced Logistic Glitch)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .bitcrush_bits = 4,
        .rnd_octave_range = 50,
        .prob_skip_score = 20,
        .steps = {
            {.wave_idx = 248, .duration_cycles = 150, .pitch_offset = 0, .flags = PX_WSEQ_BITCRUSH | PX_WSEQ_USE_RND_OCTAVE | PX_WSEQ_USE_PROB_SKIP | PX_WSEQ_REVERSE_PLAY}, // Chaotic!
            {.flags = PX_WSEQ_END}
        }
    },
    // 122: Digital Hurricane (Replaced Bitcrush Storm)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .bitcrush_bits = 2,
        .prob_skip_score = 25,
        .prob_mute_score = 10,
        .steps = {
            {.wave_idx = 245, .duration_cycles = 50, .pitch_offset = 0, .flags = PX_WSEQ_BITCRUSH | PX_WSEQ_REVERSE_PLAY | PX_WSEQ_USE_PROB_MUTE | PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 123: Neural Net (Replaced Self-Mod Glitch)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .xmod_depth = 0.7f,
        .xmod_mod_src = -1,
        .ring_mod_depth = 0.3f,
        .ring_mod_mod_src = -1,
        .steps = {
            {.wave_idx = 95, .duration_cycles = 300, .pitch_offset = 0, .flags = PX_WSEQ_XMOD | PX_WSEQ_RING_MOD | PX_WSEQ_GLIDE}, // Alien Comm
            {.wave_idx = 95, .duration_cycles = 100, .pitch_offset = 1200, .flags = PX_WSEQ_XMOD | PX_WSEQ_RING_MOD | PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 124: Quantum Leaps (Replaced Stutter Morph)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .rnd_wave_low = 50,
        .rnd_wave_high = 60,
        .steps = {
            {.wave_idx = 50, .duration_cycles = 40, .pitch_offset = 0, .flags = PX_WSEQ_USE_RND_WAVE | PX_WSEQ_RETRIG_ADSR}, // Random Phase Dist waves
            {.wave_idx = 50, .duration_cycles = 40, .pitch_offset = 500, .flags = PX_WSEQ_USE_RND_WAVE | PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        },
        .adsr_retrig_phase = 1
    },
    // 125: Singularity (Replaced Total Chaos)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .rnd_wave_low = 0,
        .rnd_wave_high = 255,
        .rnd_octave_range = 90,
        .ring_mod_depth = 0.5f,
        .ring_mod_mod_src = -1,
        .xmod_depth = 0.5f,
        .xmod_mod_src = -1,
        .steps = {
            {.wave_idx = 0, .duration_cycles = 10, .pitch_offset = 0, .flags = PX_WSEQ_USE_RND_WAVE | PX_WSEQ_USE_RND_OCTAVE | PX_WSEQ_REVERSE_PLAY | PX_WSEQ_RING_MOD | PX_WSEQ_XMOD | PX_WSEQ_BITCRUSH}, // EVERYTHING
            {.flags = PX_WSEQ_END}
        }
    },
    // 126: Time Travel (Replaced Reverse Tape)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .prob_skip_score = 30,
        .ring_mod_depth = 0.25f,
        .ring_mod_mod_src = -1,
        .steps = {
            {.wave_idx = 2, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_REVERSE_PLAY | PX_WSEQ_RING_MOD | PX_WSEQ_GLIDE | PX_WSEQ_USE_PROB_SKIP},
            {.wave_idx = 2, .duration_cycles = 200, .pitch_offset = -500, .flags = PX_WSEQ_REVERSE_PLAY | PX_WSEQ_RING_MOD | PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 127: Heat Death (Replaced The End)
    {
        .end_action = PX_WSEQ_END_STOP,
        .bitcrush_bits = 8,
        .steps = {
            {.wave_idx = 2, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_BITCRUSH}, // Degrading
            {.wave_idx = 2, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_BITCRUSH}, // ...
            {.wave_idx = 0, .duration_cycles = 0, .pitch_offset = 0, .flags = PX_WSEQ_END} // Silence
        }
    }
};
