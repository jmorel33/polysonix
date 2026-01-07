// --- v1.5 ROM (Populated) ---
static const PxWaveSequence ROM_WAVE_SEQUENCES[PX_NUM_WSEQ_BANKS] = {
    // Seq 0: Basic Loop (4 steps, Sine/Tri/Saw/Square, no pitch shift)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .glide_mode = PX_WSEQ_GLIDE_OFF,
        .prob_mute_score = 0,
        .prob_skip_score = 0,
        .steps = {
            {.wave_idx = 0, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, // Sine
            {.wave_idx = 1, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, // Tri
            {.wave_idx = 2, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, // Saw
            {.wave_idx = 3, .duration_cycles = 100, .pitch_offset = 0, .flags = 0}, // Square
            {.wave_idx = 0, .duration_cycles = 0,   .pitch_offset = 0, .flags = PX_WSEQ_END}
        }
    },
    // Seq 1: Major Triad Arp (PingPong)
    {
        .end_action = PX_WSEQ_END_PINGPONG,
        .steps = {
            {.wave_idx = 3, .duration_cycles = 200, .pitch_offset = 0,   .flags = 0},
            {.wave_idx = 3, .duration_cycles = 200, .pitch_offset = 400, .flags = 0}, // +4 st
            {.wave_idx = 3, .duration_cycles = 200, .pitch_offset = 700, .flags = 0}, // +7 st
            {.wave_idx = 0, .duration_cycles = 0,   .pitch_offset = 0,   .flags = PX_WSEQ_END}
        }
    },
    // Seq 2: Glitch Rhythmic (Prob Mute/Skip, Bitcrush)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .bitcrush_bits = 4,
        .prob_mute_score = 30, // 30% mute
        .prob_skip_score = 10, // 10% skip
        .steps = {
            {.wave_idx = 2, .duration_cycles = 50, .pitch_offset = 0, .flags = PX_WSEQ_USE_PROB_MUTE | PX_WSEQ_BITCRUSH},
            {.wave_idx = 2, .duration_cycles = 50, .pitch_offset = 0, .flags = PX_WSEQ_USE_PROB_MUTE},
            {.wave_idx = 2, .duration_cycles = 50, .pitch_offset = 0, .flags = PX_WSEQ_USE_PROB_MUTE | PX_WSEQ_BITCRUSH},
            {.wave_idx = 2, .duration_cycles = 50, .pitch_offset = 0, .flags = PX_WSEQ_USE_PROB_MUTE},
            {.wave_idx = 0, .duration_cycles = 0,  .pitch_offset = 0, .flags = PX_WSEQ_END}
        }
    },
    // Seq 3: Modulated Texture (Ring Mod + XMod)
    {
        .end_action = PX_WSEQ_END_LOOP,
        .xmod_depth = 0.5f,
        .steps = {
            {.wave_idx = 0, .duration_cycles = 500, .pitch_offset = 0,    .flags = PX_WSEQ_XMOD},
            {.wave_idx = 0, .duration_cycles = 500, .pitch_offset = 1200, .flags = PX_WSEQ_RING_MOD},
            {.wave_idx = 0, .duration_cycles = 0,   .pitch_offset = 0,    .flags = PX_WSEQ_END}
        }
    },
    // Seq 4
    {
        .end_action = PX_WSEQ_END_LOOP,
        .glide_mode = PX_WSEQ_GLIDE_OFF,
        .bitcrush_bits = 0,
        .steps = {
            { .wave_idx = 0, .duration_cycles = 100, .pitch_offset = 0, .flags = 0 },
            { .wave_idx = 1, .duration_cycles = 100, .pitch_offset = 0, .flags = 0 },
            { .wave_idx = 2, .duration_cycles = 100, .pitch_offset = 0, .flags = 0 },
            { .wave_idx = 3, .duration_cycles = 100, .pitch_offset = 0, .flags = PX_WSEQ_LOOP_POINT }
        }
    },
    // Seq 5: "Ham Crazy" Glitch
    {
        .end_action = PX_WSEQ_END_PINGPONG,
        .glide_mode = PX_WSEQ_GLIDE_SMOOTH,
        .bitcrush_bits = 4,
        .ring_mod_depth = 0.5f,
        .steps = {
            { .wave_idx = 5, .duration_cycles = 20, .pitch_offset = 0,    .flags = PX_WSEQ_BITCRUSH },
            { .wave_idx = 5, .duration_cycles = 20, .pitch_offset = 1200, .flags = PX_WSEQ_RING_MOD },
            { .wave_idx = 5, .duration_cycles = 20, .pitch_offset = 0,    .flags = PX_WSEQ_BITCRUSH | PX_WSEQ_REVERSE_PLAY },
            { .wave_idx = 5, .duration_cycles = 20, .pitch_offset = -1200,.flags = PX_WSEQ_RING_MOD }
        }
    }
};
