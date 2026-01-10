// --- v1.5 ROM (Populated) ---
// 128 Sequences organized into 16 themed banks of 8.

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
    // 2: FM Solo (Expressive)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .glide_mode = PX_WSEQ_GLIDE_STEP,
        .steps = {
            {.wave_idx = 70, .duration_cycles = 50, .pitch_offset = 0, .flags = 0}, // FM Dynamic Lead
            {.wave_idx = 70, .duration_cycles = 50, .pitch_offset = 10, .flags = 0}, // Vibrato-ish
            {.wave_idx = 70, .duration_cycles = 50, .pitch_offset = 0, .flags = 0},
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
        }
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
    // 5: Fast Arp Lead (Generative Octave Jumps)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .rnd_octave_range = 30,
        .steps = {
            {.wave_idx = 4, .duration_cycles = 150, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 4, .duration_cycles = 150, .pitch_offset = 400, .flags = PX_WSEQ_USE_RND_OCTAVE}, // Random Octave Jump
            {.wave_idx = 4, .duration_cycles = 150, .pitch_offset = 700, .flags = 0},
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
    // 9: Glassy Pad
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 71, .duration_cycles = 500, .pitch_offset = 0, .flags = 0}, // FM Glassy Evolve
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
    // 11: Additive Morph
    {
        .end_action = PX_WSEQ_END_PINGPONG,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 107, .duration_cycles = 600, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 108, .duration_cycles = 600, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 109, .duration_cycles = 600, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE | PX_WSEQ_RING_MOD}, // Added Ring Mod
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
            {.wave_idx = 9, .duration_cycles = 300, .pitch_offset = 2, .flags = PX_WSEQ_GLIDE}, // Sine/Saw
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
        .ring_mod_depth = 0.3f
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
    // 22: Crushed Strings
    {
        .end_action = PX_WSEQ_END_LOOP,
        .bitcrush_bits = 6,
        .steps = {
            {.wave_idx = 147, .duration_cycles = 200, .pitch_offset = -5, .flags = PX_WSEQ_BITCRUSH},
            {.wave_idx = 147, .duration_cycles = 200, .pitch_offset = 5, .flags = PX_WSEQ_BITCRUSH},
            {.flags = PX_WSEQ_END}
        }
    },
    // 23: Evolving Strings
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 151, .duration_cycles = 800, .pitch_offset = 0, .flags = 0}, // Morphing Harmonics
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
    // 50: HiHat
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 132, .duration_cycles = 50, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} } // Cymbalish
    },
    // 51: Tom
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = { {.wave_idx = 131, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 52: Glitch Perc
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = {
            {.wave_idx = 143, .duration_cycles = 100, .pitch_offset = 0, .flags = PX_WSEQ_RESET_LFO | PX_WSEQ_RETRIG_ADSR}, // Added Reset/Retrig
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
        .xmod_depth = 0.3f
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
    // 92: Morphing Lead
    {
        .end_action = PX_WSEQ_END_PINGPONG,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .steps = {
            {.wave_idx = 6, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.wave_idx = 4, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_GLIDE},
            {.flags = PX_WSEQ_END}
        }
    },
    // 93: Stutter
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 6, .duration_cycles = 20, .pitch_offset = 0, .flags = PX_WSEQ_USE_PROB_SKIP}, // Added skip probability
            {.wave_idx = 6, .duration_cycles = 20, .pitch_offset = 0, .flags = PX_WSEQ_USE_PROB_MUTE},
            {.flags = PX_WSEQ_END}
        },
        .prob_mute_score = 30,
        .prob_skip_score = 40
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
    // 95: Chaos Theory
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 248, .duration_cycles = 100, .pitch_offset = 0, .flags = PX_WSEQ_USE_RND_OCTAVE | PX_WSEQ_USE_RND_WAVE}, // Logistic Chaos
            {.flags = PX_WSEQ_END}
        },
        .rnd_octave_range = 50,
        .rnd_wave_low = 0,
        .rnd_wave_high = 255
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
    // 98: Chaos Drone
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 93, .duration_cycles = 1000, .pitch_offset = -1200, .flags = PX_WSEQ_RING_MOD | PX_WSEQ_XMOD}, // Heavy texture
            {.flags = PX_WSEQ_END}
        },
        .ring_mod_depth = 0.3f,
        .xmod_depth = 0.4f,
        .ring_mod_mod_src = -1,
        .xmod_mod_src = -1
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
    // 107: Laser Harp
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 50, .duration_cycles = 200, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} } // Laser Malfunction
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
    // Generative, chaotic, and experimental sequences.
    // 120: Fibonacci
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 247, .duration_cycles = 200, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 121: Logistic
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 248, .duration_cycles = 200, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 122: Noise Wall
    {
        .end_action = PX_WSEQ_END_LOOP,
        .bitcrush_bits = 2,
        .steps = { {.wave_idx = 245, .duration_cycles = 100, .pitch_offset = 0, .flags = PX_WSEQ_BITCRUSH}, {.flags = PX_WSEQ_END} }
    },
    // 123: Alien Comm
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = { {.wave_idx = 95, .duration_cycles = 300, .pitch_offset = 0, .flags = 0}, {.flags = PX_WSEQ_END} }
    },
    // 124: Broken Toy
    {
        .end_action = PX_WSEQ_END_LOOP,
        .prob_skip_score = 30,
        .steps = {
            {.wave_idx = 78, .duration_cycles = 50, .pitch_offset = 0, .flags = PX_WSEQ_USE_PROB_SKIP},
            {.wave_idx = 78, .duration_cycles = 50, .pitch_offset = 300, .flags = PX_WSEQ_USE_PROB_SKIP | PX_WSEQ_REVERSE_PLAY}, // Added Reverse
            {.flags = PX_WSEQ_END}
        }
    },
    // 125: Glitch Storm
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 0, .duration_cycles = 10, .pitch_offset = 0, .flags = PX_WSEQ_USE_RND_WAVE | PX_WSEQ_USE_RND_OCTAVE}, // Added Random Octave
            {.flags = PX_WSEQ_END}
        },
        .rnd_wave_low = 0,
        .rnd_wave_high = 255,
        .rnd_octave_range = 60
    },
    // 126: Reverse Tape
    {
        .end_action = PX_WSEQ_END_LOOP,
        .steps = {
            {.wave_idx = 2, .duration_cycles = 200, .pitch_offset = 0, .flags = PX_WSEQ_REVERSE_PLAY | PX_WSEQ_RING_MOD}, // Added Ring Mod
            {.flags = PX_WSEQ_END}
        },
        .ring_mod_depth = 0.25f,
        .ring_mod_mod_src = -1
    },
    // 127: The End
    {
        .end_action = PX_WSEQ_END_STOP,
        .steps = {
            {.wave_idx = 2, .duration_cycles = 1000, .pitch_offset = 0, .flags = 0},
            {.wave_idx = 0, .duration_cycles = 0, .pitch_offset = 0, .flags = PX_WSEQ_END} // Silence at end
        }
    }
};
