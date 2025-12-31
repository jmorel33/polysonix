#include <stdio.h>
#include <stdint.h>
#ifdef POLYSONIX_USE_GPU_WAVE
    #include "../polysonix_wave_gpu.h"
#else
    #include "../polysonix_wave_cpu.h"
#endif

/***************************************************************************************************
*
* -- Polysonix Waveform Scripting Language --  PATCHES
*   (c) 2025 Jacques Morel
*
*
****************************************************************************************************/
#define NUM_DEFAULT_WAVES 212

// --- Waveform Expressions ---
typedef struct {
    const char *name;       // Human-readable name for the wave
    const char *expression; // The mathematical expression string
    BytecodeChunk* compiled_bytecode; // Pointer to the compiled version (initially NULL)
    //float generation_rand_offset; // Stored per wave definition
} WaveDefinition;

WaveDefinition default_waves[NUM_DEFAULT_WAVES] = {
    // --- Basic Waves (0-7) ---
    /* 0*/ { "Triangle Up", "1.0 - 2.0 * abs((x + MOD_C * 0.5) / PI - 1.0) + 1.0 * MOD_B * sin(2.0*x)" }, // MOD_A: Unused. MOD_B: Harmonic. MOD_C: Phase skew/bend.
    /* 1*/ { "Triangle Down", "2.0 * abs((x + MOD_C * 0.5) / PI - 1.0) - 1.0 + 1.0 * MOD_B * sin(2.0*x)" }, // MOD_A: Unused. MOD_B: Harmonic. MOD_C: Phase skew/bend.
         // Explanation: This remains the same. Adding to 'x' is a simple phase offset, which effectively "skews" the triangle's shape. This is cheap and valid.
    /* 2*/ { "Sine Up", "((1.0 - (0.5 * (MOD_A + 1.0))) * sin(x + MOD_C * 0.25) + (0.5 * (MOD_A + 1.0)) * tanh(5.0 * sin(x + MOD_C * 0.25))) + 1.0 * MOD_B * sin(2.0*x)" }, // MOD_A: Sine-to-tanh mix. MOD_B: Harmonic. MOD_C: Phase modulation.
    /* 3*/ { "Sine Down", "((1.0 - (0.5 * (MOD_A + 1.0))) * (-sin(x + MOD_C * 0.25)) + (0.5 * (MOD_A + 1.0)) * (-tanh(5.0 * sin(x + MOD_C * 0.25)))) + 1.0 * MOD_B * sin(2.0*x)" }, // MOD_A: Sine-to-tanh mix. MOD_B: Harmonic. MOD_C: Phase modulation.
         // Explanation: This also remains the same. Adding MOD_C inside the sin() and tanh() is valid phase modulation.
    /* 4*/ { "Square Up", "(x < (TWO_PI * (0.5 + 0.4 * MOD_A)) ? 1.0 : -1.0) + 2.0 * MOD_B * sin(2.0 * x + MOD_C * PI)" }, // MOD_A: PWM. MOD_B: Harmonic. MOD_C: Phase modulation on the harmonic.
    /* 5*/ { "Square Down", "(x < (TWO_PI * (0.5 + 0.4 * MOD_A)) ? -1.0 : 1.0) + 2.0 * MOD_B * sin(2.0 * x + MOD_C * PI)" }, // MOD_A: PWM. MOD_B: Harmonic. MOD_C: Phase modulation on the harmonic.
         // Explanation: The previous suggestion was overly complex. A much simpler and still effective approach is to use MOD_C to phase-shift the harmonic sine wave added by MOD_B. This creates a moving texture against the static square wave, which is audibly distinct from PWM.
    /* 6*/ { "Saw Rising", "(1.0 - (0.5 + 0.5*MOD_A)) * ((x + x*MOD_C*0.5) / PI - 1.0) + (0.5 + 0.5*MOD_A) * (x > PI ? 1.0 : -1.0) + 2.0 * MOD_B * sin(2.0*x)" }, // MOD_A: Saw-to-pulse mix. MOD_B: Harmonic. MOD_C: Phase distortion/bend.
    /* 7*/ { "Saw Falling", "(1.0 - (0.5 + 0.5*MOD_A)) * (1.0 - (x + x*MOD_C*0.5) / PI) + (0.5 + 0.5*MOD_A) * (x > PI ? -1.0 : 1.0) + 2.0 * MOD_B * sin(2.0*x)" }, // MOD_A: Saw-to-pulse mix. MOD_B: Harmonic. MOD_C: Phase distortion/bend.
         // Explanation: My previous use of fmod() was incorrect. A valid and cheap way to create a similar "bending" or "sync-like" effect is to multiply 'x' by a term derived from MOD_C. The expression
         // `(x + x*MOD_C*0.5)` effectively speeds up or slows down the phase progression. When MOD_C is positive, the saw completes its cycle faster and stays at its peak longer. When negative, it slows down.
         // This creates a sync-like timbral change without needing fmod. I've also updated the MOD_A logic to be a clean blend between a saw and a 50% pulse wave.

    // --- Half/Half Waves (8-15) ---
    /* 8*/ { "Saw/Sine Up", "((1.0 - (0.5 * (MOD_A + 1.0))) * (x < PI ? (x / PI_OVER_2 - 1.0) : sin(x + PI * MOD_B)) + (0.5 * (MOD_A + 1.0)) * tanh(6.0 * (x < PI ? (x / PI_OVER_2 - 1.0) : sin(x + PI * MOD_B))))" }, // Phase shift on sine part
    /* 9*/ { "Sine/Saw Down", "((1.0 - (0.5 * (MOD_A + 1.0))) * (x < PI ? -sin(x + PI * MOD_B) : (1.0 - (x - PI) / PI_OVER_2)) + (0.5 * (MOD_A + 1.0)) * tanh(6.0 * (x < PI ? -sin(x + PI * MOD_B) : (1.0 - (x - PI) / PI_OVER_2))))" }, // Phase shift on sine
    /*10*/ { "Square/Sine Up", "((1.0 - (0.5 * (MOD_A + 1.0))) * (x < PI ? 1.0 : sin(x + PI * MOD_B)) + (0.5 * (MOD_A + 1.0)) * tanh(6.0 * (x < PI ? 1.0 : sin(x + PI * MOD_B))))" }, // Phase shift on sine
    /*11*/ { "Sine/Square Down", "((1.0 - (0.5 * (MOD_A + 1.0))) * (x < PI ? sin(x + PI * MOD_B) : -1.0) + (0.5 * (MOD_A + 1.0)) * tanh(6.0 * (x < PI ? sin(x + PI * MOD_B) : -1.0)))" }, // Phase shift on sine
    /*12*/ { "Saw/Triangle Up", "(x < (TWO_PI * (0.5 + 0.25 * MOD_A + 0.1 * MOD_B)) ? (x / PI_OVER_2 - 1.0) : ((x - PI) < PI_OVER_2 ? ((x - PI) / PI_OVER_2) : (1.0 - ((x - PI) - PI_OVER_2) / PI_OVER_2)))" }, // Adjust split point
    /*13*/ { "Triangle/Saw Down", "(x < (TWO_PI * (0.5 + 0.25 * MOD_A + 0.1 * MOD_B)) ? (-2.0 * abs(x / PI - 0.5)) : (3.0 - 2.0 * x / PI))" }, // Adjust split point
    /*14*/ { "Triangle/Sine Up", "((1.0 - (0.5 * (MOD_A + 1.0))) * (2.0 * (1.0 - abs(x / (PI/2) - 1.0)) - 1.0) + (0.5 * (MOD_A + 1.0)) * sin(x + PI * MOD_B))" }, // Phase shift on sine
    /*15*/ { "Sine/Triangle Down", "((1.0 - (0.5 * (MOD_A + 1.0))) * cos(x + PI + PI * MOD_B) + (0.5 * (MOD_A + 1.0)) * (-2.0 * (abs((x - PI) / PI - 0.5)) + 0.5))" }, // Phase shift on sine

    /*16*/ { "Pulse 25%", "(x < (TWO_PI * (0.25 + 0.25 * MOD_A)) ? 1.0 : -1.0) + 1 * MOD_B * sin(2*x)" }, // Harmonic softening
    /*17*/ { "Pulse 75%", "(x < (TWO_PI * (0.75 + 0.25 * MOD_A)) ? 1.0 : -1.0) + 1 * MOD_B * sin(2*x)" }, // Harmonic softening
    /*18*/ { "Clipped Sine", "min(0.5 + 0.5 * MOD_B, max(-0.5 - 0.5 * MOD_B, sin(x + 0.3 * MOD_A))) * 2.0" }, // Adjust clipping level
    /*19*/ { "Rectified Sine", "abs(sin(x + 0.4 * MOD_A)) * 2.0 - 1.0 + 1.0 * MOD_B * sin(2*x)" }, // Harmonic texture
    /*20*/ { "Staircase 4 Step", "(x < (TWO_PI * (0.125 + 0.05 * MOD_A)) ? -0.75 : (x < (TWO_PI * (0.25 + 0.05 * MOD_A)) ? -0.25 : (x < (TWO_PI * (0.375 + 0.05 * MOD_A)) ? 0.25 : 0.75))) + 0.5 * MOD_B * sin(2*x)" }, // Harmonic texture
    /*21*/ { "Sine Harmonics", "0.5 * (sin(x) + (0.5 + 0.3 * MOD_A) * sin(2*x) + 0.3*sin(3*x) + 0.5 * MOD_B * sin(4*x))" }, // Add 4th harmonic
    /*22*/ { "Formantish", "0.33 * (sin(2*x) + sin(3*x + 0.4 * MOD_A) + sin(5*x + PI * MOD_B))" }, // Phase shift on 5x harmonic
    /*23*/ { "Sine * Saw", "sin(x + 0.3 * MOD_A) * (tanh(x) * PI) + 0.5 * MOD_B * sin(2*x)" }, // Harmonic texture
    /*24*/ { "Alien Chatter", "(x < (TWO_PI * (0.15 + 0.05 * MOD_A)) ? 1.0 : (x < (TWO_PI * (0.25 + 0.05 * MOD_A)) ? -0.5 : (x < (TWO_PI * (0.35 + 0.05 * MOD_A)) ? 0.7 : (x < PI ? -1.0 : sin(3*x + 0.4 * MOD_A) * 0.6)))) + 0.5 * MOD_B * sin(2*x)" }, // Harmonic texture
    /*25*/ { "Warp Speed", "(x < (TWO_PI * 0.7) ? (x / PI_OVER_2 - 1.0) : sin(5*x + 0.5 * MOD_A + PI * MOD_B))" }, // Phase shift on high harmonic
    /*26*/ { "Overload Spark", "(x < (TWO_PI * (0.2 + 0.2 * MOD_A)) ? 1.0 : (x < (TWO_PI * (0.4 + 0.2 * MOD_A)) ? -1.0 : (x < (TWO_PI * (0.6 + 0.2 * MOD_A)) ? 0.5 : (1.0 - x / TWO_PI) * 1.5 - 0.5))) + 1.0 * MOD_B * sin(2*x)" }, // Harmonic texture
    /*27*/ { "Ghost Wail", "sin(x + 0.3 * sin(4*x + MOD_A) + PI * MOD_B)" }, // Additional phase modulation
    /*28*/ { "POKEY Pure", "(x < (TWO_PI * (0.5 + 0.1 * MOD_A)) ? (1.0 - 0.5*x/(TWO_PI*0.5)) : (-1.0 + 0.5*(x - TWO_PI*0.5)/(TWO_PI*0.5))) + 0.5 * MOD_B * sin(2*x)" }, // Harmonic texture
    /*29*/ { "Laser Malfunction", "min(1.0, max(-1.0, (x / PI - 1.0) * sin(3*x + RAND_OFFSET + 0.4 * MOD_A + PI * MOD_B)))" }, // Phase shift on modulation
    /*30*/ { "Bit Crush Bomb", "(x < (TWO_PI * (0.4 + 0.1 * MOD_A)) ? 1.0 : ((floor((x - (TWO_PI * (0.4 + 0.1 * MOD_A))) / (TWO_PI/10)) % 4) - 2) * 0.5) + 0.5 * MOD_B * sin(2*x)" }, // Harmonic texture
    /*31*/ { "Hyperspace Glitch", "(x < (TWO_PI * (0.2 + 0.05 * MOD_A)) ? 1.0 : (x < (TWO_PI * (0.4 + 0.05 * MOD_A)) ? cos(4*x + 0.4 * MOD_A + PI * MOD_B) : (x < (TWO_PI * (0.6 + 0.05 * MOD_A)) ? -1.0 : sin(6*x + 0.4 * MOD_A) * 0.7)))" }, // Phase shift on cos

    /*32*/ { "Razor Pulse", "(x < (TWO_PI * (0.05 + 0.02 * MOD_A)) ? 1.0 : (x < (TWO_PI * (0.1 + 0.02 * MOD_A)) ? -1.0 : (x / PI - 1.0))) + 0.5 * MOD_B * sin(2*x)" }, // Harmonic texture
    /*33*/ { "Shredded Saw", "min(0.8, max(-0.8, 2 * (x / PI - 1.0 + 0.2 * MOD_A * sin(3*x + PI * MOD_B))))" }, // Phase shift on modulation
    /*34*/ { "Bit-Crushed Square", "min(1.0, max(-1.0, (x < PI ? 1.0 : -1.0) + sin(2*x + RAND_OFFSET + 0.5 * MOD_A + PI * MOD_B)))" }, // Phase shift on noise
    /*35*/ { "Glitch Sine", "sin(x + 0.5 * MOD_A + PI * MOD_B) + (x > (TWO_PI*0.2) && x < PI_OVER_2 ? -0.5 : 0) + (x > (TWO_PI*0.6) && x < (TWO_PI*0.65) ? 0.5 : 0)" }, // Additional phase shift
    /*36*/ { "Overfolded Saw", "min(1.0, max(-1.0, 1.2 * (x / PI - 1.0) * sin(3*x + RAND_OFFSET + 0.3 * MOD_A + PI * MOD_B)))" }, // Phase shift on modulation
    /*37*/ { "Clipped Chaos", "min(0.5, max(-0.5, 1.5 * ((x < PI_OVER_2) ? (x / PI_OVER_2) : (x < THREE_PI_OVER_2 ? (1.0 - (x - PI_OVER_2) / PI_OVER_2) : (-1.0 + (x - THREE_PI_OVER_2) / PI_OVER_2))))) * sin(4*x + 0.4 * MOD_A + PI * MOD_B)" }, // Phase shift on modulation
    /*38*/ { "Pulse Train Wreck", "(x < (TWO_PI * (0.1 + 0.03 * MOD_A)) ? 1.0 : (x < (TWO_PI * (0.15 + 0.03 * MOD_A)) ? -1.0 : (x < (TWO_PI * (0.3 + 0.03 * MOD_A)) ? 0.5 : (x < (TWO_PI * (0.35 + 0.03 * MOD_A)) ? -0.5 : 0.0)))) + 0.2 * MOD_B * sin(2*x)" }, // Harmonic texture
    /*39*/ { "Harmonic Noise Blast", "0.5 * (sin(2*x) + sin(4.6*x + RAND_OFFSET) + MOD_A * sin(7.85*x - RAND_OFFSET*2) + 0.5 * MOD_B * sin(3*x))" }, // Add 3rd harmonic
    /*40*/ { "Wooden Percussion", "0.7 * (sin(x + 0.2 * MOD_A) + 0.5*sin(2*x) + 0.25*sin(3*x + PI * MOD_B) + 0.125*sin(4*x))" }, // Phase shift on 3rd harmonic
    /*41*/ { "Brass", "0.3 * (sin(x) + 0.8*sin(2*x + 0.3 * MOD_A) + 0.6*sin(3*x) + 0.5*sin(4*x + PI * MOD_B) + 0.4*sin(5*x) + 0.3*sin(6*x))" }, // Phase shift on 4th harmonic
    /*42*/ { "Bowed String", "0.5 * (sin(x) + sin(2*x)/2 + (0.5 + 0.3 * MOD_A) * sin(3*x)/3 + sin(4*x + PI * MOD_B)/4 + sin(5*x)/5 + sin(6*x)/6)" }, // Phase shift on 4th harmonic
    /*43*/ { "4-Segment Bump", "-1.0 + (x < PI_OVER_2 ? (1.0 * sin(2.0 * (x + 0.2 * MOD_A + PI * MOD_B))) : (x < PI ? (1.5 * sin(2.0 * (x - PI_OVER_2))) : (x < THREE_PI_OVER_2 ? (1.75 * sin(2.0 * (x - PI))) : (0.75 * sin(2.0 * (x - THREE_PI_OVER_2))))))" }, // Phase shift on first segment
    /*44*/ { "Vocal Ah", "0.35 * (0.6*sin(x) + 0.8*sin(2*x) + sin(3*x) + 0.9*sin(5*x + 0.3 * MOD_A + PI * MOD_B) + 0.2*sin(10*x))" }, // Phase shift on 5th harmonic
    /*45*/ { "Bird Call AM", "sin(x + 0.5 * MOD_A) * (0.5 + 0.5 * sin(5*x + PI * MOD_B))" }, // Phase shift on carrier
    /*46*/ { "Water Droplet", "(x < (TWO_PI*0.05) ? -1.0 : (sin((x - (TWO_PI*0.05)) * 5.0 + 1 * MOD_A + PI * MOD_B) * 0.6 * exp(-(x - (TWO_PI*0.05)) * 0.5)))" }, // Phase shift on ripple
    /*47*/ { "Wind AM", "sin(15*x + 0.75 * MOD_A + PI * MOD_B) * (0.5 + 0.5 * sin(x))" }, // Phase shift on high harmonic

    /*48*/ { "Kick Drum", "0.6 * (sin(x + 0.3 * MOD_A) + 0.4*sin(2*x) + 0.2*sin(3*x + PI * MOD_B))" }, // Phase shift on 3rd harmonic
    /*49*/ { "Snare Drum", "min(1.0, max(-1.0, (sin(5*x + 0.5 * MOD_A + PI * MOD_B) + sin(7.5*x)) * 2.0)) * (1.0 - x / (TWO_PI * 1.27))" }, // Phase shift on 5x harmonic
    /*50*/ { "Clap", "min(1.0, max(-1.0, (sin(4*x) + sin(6*x + 0.4 * MOD_A + PI * MOD_B) + sin(8*x)) * 1.5)) * (1.0 - x / (TWO_PI * 1.6))" }, // Phase shift on 6x harmonic
    /*51*/ { "Narrow", "(x < (TWO_PI * (0.2 + 0.1 * MOD_A)) ? 1.0 : (x < (TWO_PI * (0.5 + 0.05 * MOD_A + 0.1 * MOD_B)) ? -1.0 : 0.0))" }, // Adjust second pulse width
    /*52*/ { "Tom Drum", "0.7 * (sin(x + 0.3 * MOD_A + PI * MOD_B) + 0.3*sin(2*x) + 0.1*sin(3*x))" }, // Phase shift on fundamental
    /*53*/ { "Cymbalish", "0.25 * (sin(x) + sin(1.57*x) + sin(2.25*x) + MOD_A * sin(3.14*x + PI * MOD_B))" }, // Phase shift on 3.14x harmonic
    /*54*/ { "Double Waves", "max(-1.0, 0.7 * (sin(x) + 0.8 * sin(1.1*x + RAND_OFFSET)) * (1.0 - x / TWO_PI) + 0.5 * (sin(2*x) + MOD_A * sin(2.2*x + RAND_OFFSET + PI * MOD_B)))" }, // Phase shift on 2.2x
    /*55*/ { "Metal Impact", "0.25 * (sin(x) + sin(1.25*x + 0.5 * MOD_A + PI * MOD_B) + sin(1.85*x) + sin(2.1*x))" }, // Phase shift on 1.25x
    /*56*/ { "Bell Tone", "0.5 * (sin(x + PI * MOD_B) + 0.5*sin(1.05*x + 0.4 * MOD_A) + 0.3*sin(1.6*x) + 0.2*sin(2.15*x))" }, // Phase shift on fundamental
    /*57*/ { "Gritty Bass", "min(1.0, max(-1.0, 1.1 * ((x / PI - 1.0) + 0.2 * MOD_A * sin(2*x + PI * MOD_B))))" }, // Phase shift on modulation
    /*58*/ { "Additive Square", "0.8 * (sin(x) + sin(3*x)/3 + MOD_A * sin(5*x + PI * MOD_B)/5 + sin(7*x)/7)" }, // Phase shift on 5th harmonic
    /*59*/ { "Electric Pianoish", "0.6 * (sin(x) + 0.5*sin(3*x + 0.3 * MOD_A + PI * MOD_B) + 0.25*sin(5*x) + 0.125*sin(7*x))" }, // Phase shift on 3rd harmonic
    /*60*/ { "Classic Pad", "0.7 * (sin(x) + (0.5 + 0.25 * MOD_A) * sin(2*x + PI * MOD_B) + 0.25*sin(3*x) + 0.125*sin(4*x))" }, // Phase shift on 2nd harmonic
    /*61*/ { "Metallic Perc", "0.25 * (sin(x) + sin(1.5*x) + MOD_A * sin(2.5*x + PI * MOD_B) + sin(3.5*x))" }, // Phase shift on 2.5x harmonic
    /*62*/ { "Hybrid Saw*Sine", "(x / PI - 1.0) * sin(2*x + 0.5 * MOD_A + PI * MOD_B)" }, // Additional phase shift
    /*63*/ { "Quantized Saw 8", "floor((x / PI - 1.0 + 0.25 * MOD_A + 0.5 * MOD_B) * 4.0) * 0.25" }, // Adjust quantization offset

    /*64*/ { "Additive Saw (A=Harms B=Shape)", "sigma(k, 1.0, 1.0 + 8.0*abs(MOD_A), 1.0, sin(x*k) / pow(k, 1.0 + abs(MOD_B))) * 0.5" },
    /*65*/ { "PWM Synth (A=Width B=Sub)", "( (x < (TWO_PI * (0.05 + 0.9*abs(MOD_A))) ? 1.0 : -1.0) * (1.0 - 0.5*abs(MOD_B)) + sin(x*0.5)*0.5*abs(MOD_B) ) * 0.7" },
    /*66*/ { "Sigma Bell (A=Decay B=Metal)", "sigma(k, 1.0, 4.0, 1.0, sin(x * k * (1.0 + 0.2*k*abs(MOD_B))) / (k+1.0)) * exp(-x * 2.0 * (1.0 + abs(MOD_A)*4.0) / (1.0 + FREQUENCY/300.0)) * 0.7" },
    /*67*/ { "Reso Filter Sweep (A=Reso B=Cutoff)", "( (x/PI-1.0) + 0.7*sin(x * (2.0+abs(MOD_A)*5.0)) ) * 0.4 * (1.0 + tanh( 3.0 * ( MOD_B - cos(x) * (1.0 - FREQUENCY/1500.0) ) ))" },
    /*68*/ { "Plucked String (A=Damp B=Body)", "sigma(k, 1.0, 3.0 + 4.0*abs(MOD_B), 1.0, sin(x * k) * pow(0.85, k*(1.0+abs(MOD_A))) ) * exp(-x*0.5) * 0.8" },
    /*69*/ { "Formant Vowel (A=Phsr1 B=Phsr2)", "0.4 * sigma(k, 1.0, 5.0, 1.0, sin(x*k) * (1.0 + (k>1.5 && k<2.5 ? 2.0*abs(MOD_A) : 0.0) + (k>3.5 && k<4.5 ? 2.0*abs(MOD_B) : 0.0)) / (k+1.0)) * (1.0 + 0.1*(FREQUENCY/1000.0))" },
    /*70*/ { "Phase Distortion (A=Amt B=Shape)", "sin(x + (abs(x/(PI) - 1.0)*2.0 - 1.0) * (1.0 + MOD_B*0.5) * PI * abs(MOD_A) * (1.0 + FREQUENCY/600.0)) * 0.9" },
    /*71*/ { "Sigma A=End B=Decay", "sigma(k, 1.0, 1.0 + abs(MOD_A) * 5.0, 1.0, sin(x * k) / (pow(k, 1.0 + abs(MOD_B)*2.0) + 0.1)) * 0.5" },
    /*72*/ { "Noisy Pad (A=NoiseAmt B=Flt)", "( sigma(k,1.0, 4.0, 1.0, sin(x*k)/k) * (1.0-abs(MOD_A)) + (rand()-0.5)*1.5*abs(MOD_A) ) * (0.5 + 0.5*tanh(3.0*(1.0-abs(MOD_B)*(1.0 - FREQUENCY/1000.0)))) * 0.6" },
    /*73*/ { "Random Phase Additive (A=RndAmt B=Harm)", "sigma(k, 1.0, 2.0 + 6.0*abs(MOD_B), 1.0, sin(x*k + MOD_A * RAND_OFFSET * k * PI) / k) * 0.6" },
    /*74*/ { "Wavefolder Sim (A=Fold B=Bias)", "asin(sin(x* (1.0 + FREQUENCY/1000.0) + MOD_B*PI) * (1.0 + abs(MOD_A)*5.0)) / (PI/2.0) * 0.8" },
    /*75*/ { "Chaos Sine (A=ModRate B=ModAmt)", "sin(x + sin(x * (1.0 + abs(MOD_A)*8.0)) * abs(MOD_B) * 3.0)" },
    /*76*/ { "Grit Additive (A=Grit B=Tone)", "tanh( sigma(k, 1.0, 3.0+4.0*abs(MOD_B), 1.0, sin(x*k)/k ) * (1.0 + abs(MOD_A)*2.0) ) * 0.8" },
    /*77*/ { "Sync Sweep No Slant", "( sin(x * (1.0 + 5.0*abs(MOD_A))) > (MOD_B * 0.9) ? 1.0 : -1.0 ) * 0.6" },
    /*78*/ { "Sync Sweep Cos Shape", "( ( sin(x * (1.0 + 5.0*abs(MOD_A))) > (MOD_B * 0.9) ? 1.0 : -1.0 ) * 0.6 + cos(x)*0.3 )"},
    /*79*/ { "Smoothed Sync (A=SyncFreq B=Duty)", "( tanh( 10.0 * (sin(x * (1.0 + 5.0*abs(MOD_A))) - (MOD_B * 0.9)) ) * 0.7 + (x/PI-1.0)*0.2 )" },

    /*80*/ { "Limited Sync (A=SyncFreq B=Duty)", "( ( sin(x * (1.0 + 2.5*abs(MOD_A))) > (MOD_B * 0.9) ? 1.0 : -1.0 ) * 0.6 + (x/PI-1.0)*0.3 )" },
    /*81*/ { "Sync Sweep (A=SyncFreq B=Duty)", "( ( sin(x * (1.0 + 5.0*abs(MOD_A))) > (MOD_B * 0.9) ? 1.0 : -1.0 ) * 0.6 + (x/PI-1.0)*0.3 )" },
    /*82*/ { "PWM Gate", "(( (x < (PI + MOD_B*PI_OVER_2)) ^ (MOD_A > 0) ) ? 1.0 : -1.0) * 0.8" },
    /*83*/ { "Harmonic Switch", "( ( (MOD_A > 0.1) ^ (MOD_B < -0.1) ) ? (sin(x) + 0.5*sin(3*x)) : (sin(x) + 0.3*cos(2*x + PI*0.25)) ) * 0.7" },
    /*84*/ { "Phase Glitch", "sin(x + ( ((MOD_A > 0) ^ (x > PI)) * PI_OVER_2 * (0.5 + 0.5*MOD_B) ))" },
    /*85*/ { "Multi-Gate", "(x < PI_OVER_2 ? sin(2*x) : (x < PI ? ((MOD_A > 0) * cos(x - PI_OVER_2) * 1.5) : ( ((MOD_A <= 0) ^ (MOD_B > 0)) * sin(3*(x-PI)) * 0.8 ) ) ) * (1.0 - 0.2*abs(MOD_A) - 0.2*abs(MOD_B))" },
    /*86*/ { "Rich String Ensemble", "sigma(k, 1.0, 5.0 + 3.0*abs(MOD_B), 1.0, (sin(x*k + MOD_A * 0.02 * k * RAND_OFFSET) + sin(x*k*(1.0 + MOD_A*0.005) + MOD_A * 0.03 * k * (k*0.1)) + sin(x*k*(1.0 - MOD_A*0.005) - MOD_A * 0.025 * k * (k*0.13)) ) / (pow(k, 1.1 + 0.3*abs(MOD_B))) ) * 0.15 * (1.0 + 0.2*sin(x*0.5 + PI_OVER_2))" },
    /*87*/ { "Mellow Brass Section", "tanh( sigma(k, 1.0, 4.0 + 2.0*abs(MOD_A), 1.0, sin(x*k + MOD_B*0.02*sin(x*7.0)) * (1.0 - 0.2*k + abs(MOD_A)*(0.1*k - 0.05)) / (pow(k, 0.8 + 0.4*(1.0-abs(MOD_A)))) ) * (0.6 + 0.3*abs(MOD_A)) ) * 0.85" },
    /*88*/ { "Simple Minor Triad", "( sin(x) + sin(x * 1.189207 * (1.0 + MOD_A*0.005) + RAND_OFFSET*0.05) * 0.8 + sin(x * 1.498307 * (1.0 - MOD_A*0.004) + RAND_OFFSET*0.07) * 0.7 ) * (0.33 / (1.0 + abs(MOD_B*0.5))) + (MOD_B > 0.5 ? sin(x * 1.781797) * 0.2 : 0.0)" },
    /*89*/ { "Oooh Choir Formant", "sigma(k, 1.0, 8.0 + 4.0*abs(MOD_B), 1.0, ( sin(x*k + RAND_OFFSET*0.1*k) * ( (1.0 / (1.0 + pow( (k - (2.5 + MOD_A*1.5)) / (0.8 + abs(MOD_A*0.3)) , 2.0))) + (0.7 / (1.0 + pow( (k - ( ((MOD_A > 0) ^ (MOD_B > 0.3)) ? (6.0 - MOD_A*2.0) : (5.0 + MOD_A*1.0) ) ) / 1.2 , 2.0))) ) ) / pow(k, 0.7 + 0.2*abs(MOD_B)) ) * 0.25 * (1.0 + 0.15*sin(x*6.0 + RAND_OFFSET*PI))" },
    /*90*/ { "Sci-Fi Drone", "( (x/PI - 1.0 + 0.3*sin(x*11.0 + RAND_OFFSET*PI)) * 0.6 + ( ( ((MOD_A > 0) ^ (MOD_B > 0)) ? ( (x*0.5 < PI ? 0.4 : -0.4) ) : ( sin(x*0.5 + MOD_B*PI_OVER_2) * 0.4 ) ) * (0.5 + 0.5*abs(MOD_A)) ) ) * (0.4 + 0.6 * (0.5 + 0.5 * tanh( 3.0 * ( (MOD_A * 0.8 - MOD_B * 0.6) - cos(x*(0.5 + ((MOD_A > 0) ^ (MOD_B < 0) ? 1.5 : 0.5) )) ) ))) * 0.7" },
    /*91*/ { "Classic Noise Sim", " ( ( ( (x * FREQUENCY * (15.0 + MOD_A*10.0) * (1.0 + RAND_OFFSET*0.1) ) % (1.0 + abs(MOD_B)*0.3) ) - 0.5 ) * ( 0.6 + 0.4 * ( ((MOD_A > 0.1) ^ (MOD_B > 0.1)) ? sin(x * FREQUENCY * (23.0 - MOD_A*5.0) * (1.0 - RAND_OFFSET*0.05)) : cos(x * FREQUENCY * (7.0 + MOD_B*3.0)) ) ) + ( (MOD_B < -0.2) ? ( ( (x * FREQUENCY * (37.0 + MOD_A*3.0) ) % 0.8 ) - 0.4 ) * 0.3 : 0.0 ) ) * 0.55 * (1.0 - abs(MOD_A)*0.1) " },
    /*92*/ { "Distorted Pitch", "( floor( ( ( (x * FREQUENCY * (8.0 + MOD_A*24.0)) % (1.0 + abs(MOD_A)*0.7 + (RAND_OFFSET * 0.1 * (MOD_A < 0.1 ? 1.0:0.0) ) ) ) ) * (2.5 + abs(MOD_B)*1.5) ) / (2.0 + abs(MOD_B)*1.0) * (1.0 + MOD_B * 0.15 * sin(x * FREQUENCY * (31.0 + MOD_A*5.0))) ) * 0.7" },
    /*93*/ { "Jittery Inharmonic Pitch", " sigma(k, 1.0, (4.0 + abs(MOD_A)*4.0), 1.0, sin( x * FREQUENCY * (k + (MOD_B * 0.05 * (k-1.0) * ( (k%2==0)?-1:1 ) )) + (RAND_OFFSET * 0.2 * k * ( (k%3==0)?0.5:0.1 )) ) * ( (1.0 / (k + abs(MOD_B)*0.5)) * (0.8 + RAND_OFFSET*0.2*MOD_A) ) ) * (0.6 / (1.0 + abs(MOD_A)*0.5))"},
    /*94*/ { "Gritty Rumble Noise", "( (rand()*2.0-1.0) * 0.4 + (rand()*2.0-1.0) * sin(x*0.1 + RAND_OFFSET*TWO_PI) * 0.3 + (rand()*2.0-1.0) * sin(x*0.5 + RAND_OFFSET*TWO_PI*0.5) * 0.2 ) * (1.0 + MOD_A*0.5)"},
    /*95*/ { "Filtered Static Noise", "sigma(k, 0.0, 7.0, 1.0, ( (rand()*2.0-1.0) * pow(0.6 + MOD_A*0.35, k) ) ) * 0.125 + ( (rand()*2.0-1.0) * sin(x * (100.0 + MOD_B*900.0) / (FREQUENCY+0.01) ) * MOD_B * 0.2 )"},

    /*96*/ { "Classic FM EP (A=Index B=Detune)", "(sin(x + sin(x * 2.0 + 0.01*MOD_B) * (1.0 + 5.0*abs(MOD_A)) * (FREQUENCY / 440.0)) + sin(x)*0.1*abs(MOD_B)) / 1.1" },
    /*97*/ { "FM Bass Growl (A=Fdbk B=Index)", "tanh( sin(x + sin(x*0.5 + 0.05*MOD_B) * MOD_B * 3.0 * (1.0+FREQUENCY/200.0) ) * (1.0 + abs(MOD_A)*2.0) ) * 0.9" },
    /*98*/ { "Freq Shifter FM (A=Shift B=Index)", "sin(x * (1.0 + MOD_A * (FREQUENCY/440.0)) + sin(x*1.5)*abs(MOD_B)*3.0)" },
    /*99*/ { "Complex FM A=Index B=ModFreq", "sin(x + sin(x * (1.0 + abs(MOD_B)*4.0)) * (FREQUENCY / 220.0) * (1.0 + MOD_A * 3.0))" },
    /*100*/{ "FM Pluck", "sin(x * (1.0 + 0.001*FREQUENCY/100.0) + sin(x * ( ((MOD_A > 0.2) ^ (MOD_B < -0.2)) ? (3.5 + MOD_A*2.0) : (1.5 - MOD_B*1.0) ) ) * (1.5 + 2.0*abs(MOD_A) + 1.0*abs(MOD_B)) * exp(-x* (2.0 + abs(MOD_B)*3.0)) ) * 0.9 * exp(-x*0.3)" },
    /*101*/{ "FM Pitched Grit", "tanh( sin( x * FREQUENCY + (2.0 + MOD_A * 3.0) * sin(x * FREQUENCY * (1.414 + MOD_B * 0.5 + RAND_OFFSET*0.05)) ) * (1.0 + MOD_A * 0.5) ) * 0.9" },
    /*102*/{ "FM Dynamic Lead", "sin(x + ( sin(x * (1.5 + MOD_A*1.5)) * (2.0 + MOD_B*2.0) ) ) + 0.5 * sin(x*2.0 + ( sin(x*2.0 * (1.5 + MOD_A*1.5)) * (2.0 + MOD_B*2.0) ) ) + 0.33 * sin(x*3.0 + ( sin(x*3.0 * (1.5 + MOD_A*1.5)) * (2.0 + MOD_B*2.0) ) )" },
    /*103*/{ "FM Glassy Evolve", "sin(x * (1.0 + RAND_OFFSET*0.005) + ( sin(x * (3.01 + RAND_OFFSET*0.01) + ( sin(x * (7.03 + RAND_OFFSET*0.02)) * (0.4 + MOD_A*0.6) ) ) * (1.0 + MOD_B*1.5) ) )" },
    /*104*/{ "FM Metallic Bell (A=Decay B=Ratio)", "sin(x + sin(x * (4.03 + MOD_B*2.0 + RAND_OFFSET*0.02)) * (3.0 + abs(MOD_A)*4.0) * exp(-x * (2.0 + abs(MOD_A)*8.0)) ) * exp(-x*0.5)" },
    /*105*/{ "FM Hollow Drone (A=ModMix B=ModRatio)", "( (sin(x + sin(x*0.75) * 3.5) * (1.0 - abs(MOD_A))) + (sin(x + sin(x*(2.5 + MOD_B*2.0)) * 1.5) * abs(MOD_A)) ) * 0.7" },
    /*106*/{ "FM Harsh Noise Sweep (A=Sweep B=Intensity)", "sin(x + tan(x * (10.0 + MOD_A*20.0 + x*5.0 )) * (0.5 + abs(MOD_B)*2.5) ) * 0.8" },
    /*107*/{ "FM Soft Pad (A=Brightness B=Chorus)", "( sin(x + sin(x*1.99) * (0.5 + MOD_A*1.5)) + sin(x*(1.0 + (RAND_OFFSET-0.5)*0.002*abs(MOD_B)) + sin(x*1.99*(1.0 + (RAND_OFFSET-0.5)*0.003*abs(MOD_B))) * (0.5 + MOD_A*1.5)) ) * 0.45" },
    /*108*/{ "FM Bipolar Sweep Pad", "sin(x + sin(x * (3.0 + MOD_A * 2.5)) * (1.5 + (MOD_B * (MOD_B > 0.0 ? 1.0 : 0.3)) * 2.0) + (MOD_B < -0.1 ? (sin(x * 0.49) * (abs(MOD_B) * 2.5)) : 0.0) ) * 0.75 * (1.0 - abs(MOD_A)*0.2)" },
    /*109*/{ "FM Clangorous Hit (A=Metal B=Dissonance)", "tanh( sin(x + sin(x * (1.414 + MOD_B*1.5)) * (1.0 + abs(MOD_B)*2.0) + sin(x * 4.75) * ((MOD_A+1.0)*0.5 * 1.5) ) * (1.0 + ((MOD_A+1.0)*0.5*0.3) ) ) * 0.85" },
    /*110*/{ "FM Breathy Flute (A=Air B=PitchMod)", "sin(x*(1.0 + sin(x*0.2 + RAND_OFFSET*TWO_PI)*0.005*abs(MOD_B)) + ( sin(x*2.95) + (rand()*2.0-1.0)*abs(MOD_A)*0.7 ) * 1.2 ) * 0.8" },
    /*111*/{ "FM Evolving SciFi (A=Evolve B=Harmonics)", "sin(x + sin(x * (1.5 + sin(x*0.05 + RAND_OFFSET)* (1.0+MOD_A*2.0) )) * 2.0 + sin(x * (6.0 + RAND_OFFSET*0.1)) * (abs(MOD_B)*1.5) ) * 0.75" },

    // LFSR Examples (112-127)
    /*112*/ { "LFSR Rhythm Gate", "sin(x) * lfsr_clock(LFSR_8BIT, 0.3 + 0.4 * MOD_A) + 0.2 * lfsr_noise(LFSR_4BIT, 2.0 + 3.0 * MOD_B)" },
    /*113*/ { "LFSR Harmonic Chaos", "sigma(k, 1.0, 6.0, 1.0, sin(x * k + lfsr_val(LFSR_12BIT, x / TWO_PI + k * 0.1, RAND_OFFSET) * PI * MOD_A) / k) * (0.4 + 0.3 * lfsr_noise(LFSR_6BIT, 0.5 + 1.5 * MOD_B))" },
    /*114*/ { "LFSR Digital Texture", "tanh(2.0 * (lfsr_val(LFSR_15BIT, x / PI, MOD_A + RAND_OFFSET) * 2.0 - 1.0 + 0.5 * sin(x + lfsr_clock(LFSR_7BIT, 0.5) * PI_OVER_2) + 0.3 * lfsr_noise(LFSR_5BIT, 4.0 + 6.0 * MOD_B)))" },
    /*115*/ { "LFSR Poly Rhythm", "sin(x) * (0.7 + 0.3 * lfsr_clock(LFSR_11BIT, 0.25)) + sin(x * 1.5) * (0.6 + 0.4 * lfsr_clock(LFSR_9BIT, 0.33 + 0.3 * MOD_A)) + 0.3 * lfsr_noise(LFSR_13BIT, 1.0 + 2.0 * MOD_B)" },
    /*116*/ { "LFSR Phase Modulation", "sin(x + lfsr_val(LFSR_16BIT, x / TWO_PI + FREQUENCY / 1000.0, RAND_OFFSET) * MOD_A * TWO_PI + lfsr_noise(LFSR_8BIT, 0.1 + MOD_B) * 0.5)" },
    /*117*/ { "LFSR Granular", "sigma(k, 1.0, 4.0, 1.0, sin(x * k) * lfsr_clock(LFSR_10BIT, 0.1 + k * 0.15) * lfsr_val(LFSR_14BIT, x / PI + k * MOD_A, RAND_OFFSET)) * 0.6" },
    /*118*/ { "LFSR Rhythmic Harmonics", "sigma(k, 1.0, 8.0, 1.0, sin(x * k) * lfsr_clock((k % 3 == 0) ? LFSR_7BIT : LFSR_5BIT, 0.2 + MOD_A * 0.3) / k) * (0.5 + 0.3 * lfsr_noise(LFSR_12BIT, 0.8 + MOD_B))" },
    /*119*/ { "LFSR Spectral Shift", "sin(x * (1.0 + lfsr_val(LFSR_13BIT, x / PI + FREQUENCY / 500.0, MOD_A) * 0.5)) + 0.4 * sin(x * 2.0 + lfsr_noise(LFSR_9BIT, 2.0 + MOD_B * 3.0) * PI)" },
    /*120*/ { "LFSR Euclidean Beat", "sin(x) * (lfsr_clock(LFSR_16BIT, 0.4) && lfsr_clock(LFSR_11BIT, 0.3 + MOD_A * 0.4) ? 1.0 : 0.3) + 0.2 * lfsr_noise(LFSR_6BIT, 3.0 + MOD_B * 2.0)" },
    /*121*/ { "LFSR Feedback Synth", "tanh(sin(x + lfsr_val(LFSR_15BIT, x / TWO_PI, RAND_OFFSET) * MOD_A * PI) + 0.3 * lfsr_noise(LFSR_8BIT, 1.5 + MOD_B * 2.0) * sin(x * 0.5)) * 0.8" },
    /*122*/ { "LFSR Algorithmic Lead", "sin(x * (1.0 + lfsr_clock(LFSR_12BIT, 0.25) * 0.5)) + sin(x * 2.0) * lfsr_val(LFSR_10BIT, x / PI + MOD_A, RAND_OFFSET) + 0.3 * lfsr_noise(LFSR_7BIT, 4.0 + MOD_B * 3.0)" },
    /*123*/ { "LFSR Morphing Pad", "sigma(k, 1.0, 5.0 + 3.0 * abs(MOD_B), 1.0, sin(x * k + lfsr_val(LFSR_14BIT, x / TWO_PI + k * 0.1, RAND_OFFSET + MOD_A) * PI) * lfsr_clock(LFSR_8BIT, 0.1 + k * 0.05) / (k + 1.0)) * 0.4" },
    /*124*/ { "LFSR Breakbeat", "sin(x) * (lfsr_clock(LFSR_9BIT, 0.5) ? 1.0 : (lfsr_clock(LFSR_7BIT, 0.7 + MOD_A * 0.2) ? 0.6 : 0.2)) + 0.4 * lfsr_noise(LFSR_11BIT, 2.5 + MOD_B * 2.0) * sin(x * 3.0)" },
    /*125*/ { "LFSR Probability Gate", "sin(x + MOD_A * PI) * (lfsr_val(LFSR_16BIT, x / PI + FREQUENCY / 440.0, RAND_OFFSET) > (0.3 + MOD_B * 0.4) ? 1.0 : 0.1) + 0.3 * lfsr_noise(LFSR_6BIT, 3.0)" },
    /*126*/ { "LFSR Polyrhythmic Chaos", "sin(x) * lfsr_clock(LFSR_13BIT, 0.3) + sin(x * 1.333) * lfsr_clock(LFSR_11BIT, 0.4 + MOD_A * 0.3) + sin(x * 1.666) * lfsr_clock(LFSR_9BIT, 0.35 + MOD_B * 0.25) * 0.8" },
    /*127*/ { "LFSR Glitch Matrix", "tanh(sigma(k, 1.0, 4.0, 1.0, sin(x * k) * lfsr_val(3, x / TWO_PI + k * MOD_A * 0.1, RAND_OFFSET) * lfsr_clock(0, 0.2 + k * 0.1)) * (1.0 + MOD_B * 2.0)) * 0.7" },

    // Atari 8-bit POKEY Sounds (128-143)
    /*128*/ { "POKEY Pure Tone", "(x < (TWO_PI * (0.5 + 0.3 * MOD_A)) ? 1.0 : -1.0) + 0.1 * (floor(x * FREQUENCY / 100.0) % 2 == 0 ? 0.3 : -0.3) * MOD_B" },
    /*129*/ { "POKEY Filtered Noise", "((floor(x * FREQUENCY * (15.0 + MOD_A * 10.0) / 64000.0) % 2) * 2.0 - 1.0) * (0.8 + 0.2 * sin(x * (2.0 + MOD_B * 3.0)))" },
    /*130*/ { "POKEY Distorted Bass", "tanh(3.0 * ((x < (TWO_PI * (0.3 + 0.4 * MOD_A)) ? 1.0 : -1.0) + 0.3 * sin(x * (1.0 + MOD_B * 2.0)))) * 0.8" },
    /*131*/ { "POKEY Laser Zap", "sin(x * (1.0 + MOD_A * 10.0) + sin(x * (31.0 + MOD_B * 20.0)) * 2.0) * exp(-x * 0.8) * (1.0 + 0.5 * ((floor(x * FREQUENCY / 1000.0) % 4) - 1.5))" },
    /*132*/ { "POKEY Explosion", "((floor(x * FREQUENCY * (50.0 + MOD_A * 100.0) / 64000.0) % 7) - 3) * 0.3 * exp(-x * (0.5 + MOD_B * 2.0)) + sin(x * 0.1) * 0.4 * exp(-x * 0.3)" },
    /*133*/ { "POKEY Engine Rumble", "sin(x * 0.25 + sin(x * 0.5) * 0.5) * 0.6 + ((floor(x * FREQUENCY * (8.0 + MOD_A * 12.0) / 31000.0) % 3) - 1) * 0.4 * (0.7 + 0.3 * sin(x * (0.1 + MOD_B * 0.2)))" },
    /*134*/ { "POKEY Bit Crush Lead", "floor(sin(x + sin(x * (2.0 + MOD_A * 4.0)) * 1.5) * (4.0 + MOD_B * 8.0)) / (4.0 + MOD_B * 8.0) * 0.9" },
    /*135*/ { "POKEY Coin Pickup", "sin(x * (1.0 + floor((x * 8.0 / TWO_PI)) * 0.5 + MOD_A * 2.0)) * exp(-x * (2.0 + MOD_B * 3.0)) * (1.0 + 0.3 * sin(x * 15.0))" },
    /*136*/ { "POKEY Jump Sound", "sin(x * (2.0 + MOD_A * 3.0) + sin(x * (8.0 + MOD_B * 6.0)) * 0.8) * exp(-x * 1.5) * (1.0 + 0.4 * ((floor(x * 20.0) % 2) * 2.0 - 1.0))" },
    /*137*/ { "POKEY Chirp Bird", "sin(x * (0.5 + x * (2.0 + MOD_A * 4.0) / TWO_PI)) * exp(-x * (1.0 + MOD_B * 2.0)) + 0.2 * ((floor(x * FREQUENCY / 5000.0) % 5) - 2) * 0.1" },
    /*138*/ { "POKEY Alien Voice", "tanh(sin(x * (0.8 + MOD_A * 1.5) + sin(x * (17.0 + MOD_B * 10.0)) * 1.2) * 2.0) * 0.7 + 0.2 * ((floor(x * FREQUENCY / 8000.0) % 8) - 3.5) * 0.1" },
    /*139*/ { "POKEY Power Up", "sin(x * (0.5 + x * (4.0 + MOD_A * 6.0) / TWO_PI + sin(x * (25.0 + MOD_B * 15.0)) * 0.3)) * exp(-x * 0.4) * (1.0 + 0.3 * sin(x * 12.0))" },
    /*140*/ { "POKEY Hit Sound", "sin(x * (2.0 + MOD_A * 2.0)) * exp(-x * (5.0 + MOD_B * 5.0)) + ((floor(x * FREQUENCY * 100.0 / 64000.0) % 16) - 8) * 0.1 * exp(-x * 3.0)" },
    /*141*/ { "POKEY Sweep Down", "sin(x * (3.0 + MOD_A * 2.0 - x * (2.0 + MOD_B * 3.0) / TWO_PI)) * exp(-x * 0.6) * (0.8 + 0.2 * ((floor(x * 10.0) % 3) - 1))" },
    /*142*/ { "POKEY Poly Counter", "((floor(x * FREQUENCY * (31.0 + MOD_A * 20.0) / 64000.0) % 31) > (15 + MOD_B * 10) ? 1.0 : -1.0) * 0.7 + 0.2 * sin(x * (0.5 + MOD_A * 0.5))" },
    /*143*/ { "POKEY Four Channel", "0.25 * (sin(x) + sin(x * 1.33 + MOD_A * PI) + ((x < PI ? 1.0 : -1.0) * (0.7 + 0.3 * MOD_B)) + ((floor(x * FREQUENCY * 17.0 / 64000.0) % 2) * 2.0 - 1.0) * 0.8)" },

    // Classic Arcade Sounds (144-175)
    /*144*/ { "Pac-Man Wakka", "sin(x * (1.0 + 0.3 * sin(x * 8.0 + MOD_A * PI))) * (x < (PI * (0.6 + 0.2 * MOD_B)) ? 1.0 : 0.3) * 0.8" },
    /*145*/ { "Pac-Man Power Pellet", "sin(x * (2.0 + sin(x * 12.0) * 1.5 + MOD_A * 2.0)) * (0.7 + 0.3 * sin(x * (4.0 + MOD_B * 4.0))) * 0.9" },
    /*146*/ { "Pac-Man Death", "sin(x * (2.0 - x * (3.0 + MOD_A * 2.0) / TWO_PI)) * exp(-x * (0.8 + MOD_B * 1.0)) * (1.0 + 0.4 * sin(x * 15.0))" },
    /*147*/ { "Pac-Man Ghost", "sin(x * (0.8 + 0.2 * sin(x * (6.0 + MOD_A * 4.0)))) * (0.6 + 0.4 * sin(x * (0.5 + MOD_B * 1.0))) * 0.7" },
    /*148*/ { "Space Invaders Shot", "sin(x * (3.0 + MOD_A * 2.0) + sin(x * (25.0 + MOD_B * 15.0)) * 0.8) * exp(-x * 2.5) * (1.0 + 0.3 * sin(x * 40.0))" },
    /*149*/ { "Space Invaders March", "((x < (TWO_PI * 0.25) ? 0.8 : (x < (TWO_PI * 0.5) ? 0.4 : (x < (TWO_PI * 0.75) ? 0.6 : 0.2))) + 0.3 * sin(x * (2.0 + MOD_A)) * (0.8 + 0.2 * MOD_B)) * 0.9" },
    /*150*/ { "Space Invaders UFO", "sin(x * (0.3 + 0.1 * sin(x * (3.0 + MOD_A * 2.0)))) + 0.4 * sin(x * (8.0 + MOD_B * 4.0) + sin(x * 0.2) * 2.0) * 0.8" },
    /*151*/ { "Space Invaders Explosion", "((floor(x * FREQUENCY * (80.0 + MOD_A * 40.0) / 22050.0) % 31) - 15) * 0.1 * exp(-x * (1.5 + MOD_B * 2.0)) + sin(x * 0.2) * 0.3 * exp(-x * 0.8)" },
    /*152*/ { "Asteroids Thrust", "sin(x * (0.5 + 0.3 * sin(x * (12.0 + MOD_A * 8.0)))) * (0.4 + 0.6 * ((floor(x * 20.0) % 3) > 0 ? 1.0 : 0.3)) + 0.2 * ((floor(x * FREQUENCY / 1000.0) % 7) - 3) * 0.1 * MOD_B" },
    /*153*/ { "Asteroids Shoot", "sin(x * (4.0 + MOD_A * 3.0) + sin(x * (35.0 + MOD_B * 20.0)) * 1.2) * exp(-x * 3.0) * (1.0 + 0.5 * sin(x * 60.0))" },
    /*154*/ { "Asteroids Explosion", "((floor(x * FREQUENCY * (120.0 + MOD_A * 80.0) / 22050.0) % 127) - 63) * 0.02 * exp(-x * (1.0 + MOD_B * 1.5)) + sin(x * (0.1 + x * 2.0 / TWO_PI)) * 0.4 * exp(-x * 0.5)" },
    /*155*/ { "Asteroids Hyperspace", "sin(x * (2.0 + x * (8.0 + MOD_A * 4.0) / TWO_PI) + sin(x * (45.0 + MOD_B * 25.0)) * 2.0) * exp(-x * 0.8) * (1.0 + 0.6 * sin(x * 80.0))" },
    /*156*/ { "Galaxian Attack", "sin(x * (1.5 + sin(x * (6.0 + MOD_A * 4.0)) * 0.8 + x * (2.0 + MOD_B * 2.0) / TWO_PI)) * exp(-x * 1.2) * 0.9" },
    /*157*/ { "Galaxian Formation", "sin(x * (0.8 + 0.2 * sin(x * (4.0 + MOD_A * 2.0)))) + 0.3 * sin(x * (2.4 + 0.4 * sin(x * (1.0 + MOD_B)))) * 0.8" },
    /*158*/ { "Centipede Laser", "sin(x * (5.0 + MOD_A * 3.0) + sin(x * (40.0 + MOD_B * 20.0)) * 0.6) * exp(-x * 2.8) * (1.0 + 0.4 * ((floor(x * 30.0) % 2) * 2.0 - 1.0))" },
    /*159*/ { "Centipede Flea Drop", "sin(x * (1.2 + x * (1.5 + MOD_A * 2.0) / TWO_PI) + sin(x * (18.0 + MOD_B * 10.0)) * 1.0) * exp(-x * 1.0) * 0.8" },
    /*160*/ { "Defender Thrust", "sin(x * (0.4 + 0.3 * sin(x * (8.0 + MOD_A * 6.0)))) * (0.5 + 0.5 * ((floor(x * 15.0) % 5) > 2 ? 1.0 : 0.4)) + 0.3 * ((floor(x * FREQUENCY / 800.0) % 15) - 7) * 0.02 * MOD_B" },
    /*161*/ { "Defender Smart Bomb", "sin(x * (2.0 + x * 4.0 / TWO_PI + sin(x * (30.0 + MOD_A * 15.0)) * 3.0)) * exp(-x * (0.6 + MOD_B * 0.8)) * (1.0 + 0.5 * sin(x * 100.0))" },
    /*162*/ { "Frogger Hop", "sin(x * (1.8 + MOD_A * 1.0) + sin(x * (12.0 + MOD_B * 8.0)) * 1.5) * exp(-x * 2.0) * (1.0 + 0.3 * sin(x * 25.0)) * 0.9" },
    /*163*/ { "Frogger Traffic", "sin(x * (0.6 + 0.1 * sin(x * (2.0 + MOD_A)))) + 0.2 * ((floor(x * FREQUENCY / 1200.0) % 11) - 5) * 0.1 * (0.8 + 0.2 * MOD_B)" },
    /*164*/ { "Donkey Kong Hammer", "sin(x * (1.5 + MOD_A * 1.5)) * exp(-x * (3.0 + MOD_B * 2.0)) + 0.4 * ((floor(x * FREQUENCY / 600.0) % 8) - 3.5) * 0.1 * exp(-x * 1.5)" },
    /*165*/ { "Donkey Kong Jump", "sin(x * (2.5 + x * (2.0 + MOD_A * 3.0) / TWO_PI) + sin(x * (20.0 + MOD_B * 10.0)) * 0.8) * exp(-x * 1.8) * 0.9" },
    /*166*/ { "Missile Command Explosion", "((floor(x * FREQUENCY * (90.0 + MOD_A * 60.0) / 22050.0) % 63) - 31) * 0.05 * exp(-x * (1.2 + MOD_B * 1.0)) + sin(x * (0.15 + x * 1.5 / TWO_PI)) * 0.5 * exp(-x * 0.7)" },
    /*167*/ { "Tempest Shoot", "sin(x * (4.5 + MOD_A * 2.5) + sin(x * (50.0 + MOD_B * 25.0)) * 0.7) * exp(-x * 3.5) * (1.0 + 0.4 * sin(x * 70.0)) * 0.8" },
    /*168*/ { "Tempest Flip", "sin(x * (3.0 + sin(x * (15.0 + MOD_A * 10.0)) * 2.0 + x * (1.0 + MOD_B) / TWO_PI)) * exp(-x * 1.5) * 0.9" },
    /*169*/ { "Berzerk Robot Voice", "tanh(sin(x * (1.2 + MOD_A * 0.8) + sin(x * (8.0 + MOD_B * 6.0)) * 1.5) * 2.5) * (0.7 + 0.3 * ((floor(x * 10.0) % 4) > 1 ? 1.0 : 0.5)) * 0.8" },
    /*170*/ { "Robotron Shoot", "sin(x * (6.0 + MOD_A * 4.0) + sin(x * (45.0 + MOD_B * 30.0)) * 0.5) * exp(-x * 4.0) * (1.0 + 0.3 * ((floor(x * 40.0) % 2) * 2.0 - 1.0))" },
    /*171*/ { "Phoenix Bird Cry", "sin(x * (0.9 + 0.4 * sin(x * (3.0 + MOD_A * 2.0)) + x * (1.5 + MOD_B) / TWO_PI)) * exp(-x * 0.9) * (1.0 + 0.2 * sin(x * 12.0)) * 0.8" },
    /*172*/ { "Gorf Laser", "sin(x * (3.5 + MOD_A * 2.0) + sin(x * (28.0 + MOD_B * 15.0)) * 1.0) * exp(-x * 2.2) * (1.0 + 0.5 * sin(x * 35.0)) * 0.9" },
    /*173*/ { "Scramble Engine", "sin(x * (0.7 + 0.2 * sin(x * (5.0 + MOD_A * 3.0)))) * (0.6 + 0.4 * ((floor(x * 12.0) % 6) > 2 ? 1.0 : 0.5)) + 0.25 * ((floor(x * FREQUENCY / 900.0) % 9) - 4) * 0.05 * MOD_B" },
    /*174*/ { "Zaxxon Alarm", "sin(x * (2.2 + 0.8 * sin(x * (1.5 + MOD_A)) + 0.3 * sin(x * (7.0 + MOD_B * 4.0)))) * (0.8 + 0.2 * sin(x * 0.8)) * 0.9" },
    /*175*/ { "Moon Patrol Bounce", "sin(x * (1.6 + MOD_A * 1.2) + sin(x * (14.0 + MOD_B * 8.0)) * 1.2) * exp(-x * (1.5 + abs(sin(x * 2.0)) * 1.0)) * (1.0 + 0.3 * sin(x * 20.0)) * 0.8" },

// POKEY Noise Emulation using LFSR (176-191) - REVISED FOR MORE AUTHENTIC POKEY CLOCKING
    // Note: FREQUENCY is the note's frequency. We need to divide by it.
    // To avoid division by zero if FREQUENCY is 0 (e.g., during table gen), add a small epsilon.
    // Let POKEY_MASTER_CLK_DIV_N be a script constant or calculated from FREQUENCY.
    // The "rate" argument to lfsr_noise now means "how many times to cycle the LFSR per x cycle"
    // So, rate = (TargetPokeyNoiseClockHz / NoteFrequencyHz) / LfsrTablePeriod

    // Define target POKEY clock rates (approx)
    // POKEY_CLK_1790K = 1789773.0 (not usually used directly for noise)
    // POKEY_CLK_64K   = 63920.0
    // POKEY_CLK_15K   = 15699.0

    // Periods for different LFSRs (as per your LfsrType enum and lfsr_configs)
    // LFSR_4BIT_PERIOD = 15.0
    // LFSR_5BIT_PERIOD = 31.0
    // LFSR_9BIT_PERIOD = 511.0
    // LFSR_17BIT_PERIOD = 131071.0 (or 65535.0 if using LFSR_16BIT for it)

    /*176*/ { "POKEY 4-bit Noise (64k)", "lfsr_noise(LFSR_4BIT, (63920.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 15.0)) * (1.0 + 0.5*abs(MOD_B)) ) * (0.6 + 0.39 * MOD_A)" },
        // MOD_A: Volume. MOD_B: Fine tune rate around 64k.

    /*177*/ { "POKEY 5-bit Noise (64k)", "lfsr_noise(LFSR_5BIT, (63920.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 31.0)) * (1.0 + 0.5*abs(MOD_B)) ) * (0.6 + 0.39 * MOD_A)" },
        // MOD_A: Volume. MOD_B: Fine tune rate.

    /*178*/ { "POKEY 17-bit Noise (64k)", "lfsr_noise(LFSR_17BIT, (63920.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 131071.0)) * (1.0 + 0.5*abs(MOD_B)) ) * (0.6 + 0.39 * MOD_A)" },
        // Assumes LFSR_17BIT has period 131071. If using LFSR_16BIT (period 65535), change 131071.0 to 65535.0

    /*179*/ { "POKEY 9-bit Noise (15k)", "lfsr_noise(LFSR_9BIT, (15699.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 511.0)) * (1.0 + 0.5*abs(MOD_B)) ) * (0.6 + 0.39 * MOD_A)" },
        // Typically 9-bit noise is clocked slower.

    /*180*/ { "POKEY Filtered 4-bit (Fast)", "lfsr_noise(LFSR_4BIT, (63920.0*2.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 15.0)) * (1.0 + 1.5*abs(MOD_B)) ) * (0.5 + 0.35 * MOD_A)" },
        // "Filtered" by making it effectively run faster than standard 64k.

    /*181*/ { "POKEY Filtered 5-bit (Fast)", "lfsr_noise(LFSR_5BIT, (63920.0*2.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 31.0)) * (1.0 + 1.5*abs(MOD_B)) ) * (0.5 + 0.35 * MOD_A)" },

    /*182*/ { "POKEY Tone + 4-bit (64k)", "( (x < (PI * (1.0 + 0.9*MOD_B)) ? 0.6 : -0.6) * (1.0 - abs(MOD_A*0.9)) + lfsr_noise(LFSR_4BIT, (63920.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 15.0)) * (1.0 + RAND_OFFSET*0.2) ) * abs(MOD_A*0.9) )" },
        // MOD_A: Mix. MOD_B: PWM. Noise rate slightly randomized around 64k.

    /*183*/ { "POKEY Tone + 5-bit (64k)", "( (x < (PI * (1.0 + 0.9*MOD_B)) ? 0.6 : -0.6) * (1.0 - abs(MOD_A*0.9)) + lfsr_noise(LFSR_5BIT, (63920.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 31.0)) * (1.0 + RAND_OFFSET*0.2) ) * abs(MOD_A*0.9) )" },

    /*184*/ { "POKEY Tone + 17-bit (64k)", "( (x < (PI * (1.0 + 0.9*MOD_B)) ? 0.6 : -0.6) * (1.0 - abs(MOD_A*0.9)) + lfsr_noise(LFSR_17BIT, (63920.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 131071.0)) * (1.0 + RAND_OFFSET*0.2) ) * abs(MOD_A*0.9) )" },
        // Again, adjust 131071.0 if using LFSR_16BIT with period 65535.

    /*185*/ { "POKEY 4(64k)+5(15k) Combined", "(lfsr_noise(LFSR_4BIT, (63920.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 15.0)) * (1.0 + RAND_OFFSET*0.1) ) * (0.5 + 0.48 * MOD_A) + lfsr_noise(LFSR_5BIT, (15699.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 31.0)) * (1.0 + RAND_OFFSET*0.1) ) * (0.5 - 0.48 * MOD_A)) * (0.55 + 0.35 * abs(MOD_B))" },
        // Mix 4-bit at ~64k and 5-bit at ~15k. MOD_A: Mix. MOD_B: Volume.

    /*186*/ { "POKEY \"High Pass\" 4-bit (Fast)", "lfsr_noise(LFSR_4BIT, (63920.0*3.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 15.0)) * (1.0 + 2.0*abs(MOD_A)) ) * (0.5 + 0.35 * abs(MOD_B))" },
        // High rate. MOD_A varies rate further. MOD_B volume.

    /*187*/ { "POKEY 64kHz Noise (17-bit)", "lfsr_noise(LFSR_17BIT, (63920.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 131071.0)) * (1.0 + 0.5*abs(MOD_A)) ) * (0.6 + 0.39 * abs(MOD_B))" },
        // This is essentially same as 178, just named differently for clarity.

    /*188*/ { "POKEY 15kHz Noise (9-bit)", "lfsr_noise(LFSR_9BIT, (15699.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 511.0)) * (1.0 + 0.5*abs(MOD_A)) ) * (0.6 + 0.39 * abs(MOD_B))" },
        // This is essentially same as 179.

    /*189*/ { "POKEY Engine Sound (Noise Gated)", "lfsr_noise(LFSR_5BIT, (15699.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 31.0)) * (1.0 + 2.0*abs(MOD_A)) ) * (0.2 + 0.7 * lfsr_val(LFSR_4BIT, (x / TWO_PI) * ((15699.0*0.5 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 15.0)) * (1.0 + 3.0*abs(MOD_B))), RAND_OFFSET)) * 0.8" },
        // 5-bit noise at ~15k (rate mod by A). Gated by 4-bit LFSR also running relative to a POKEY-like clock (rate mod by B).

    /*190*/ { "POKEY Explosion (Decaying Rate/Vol)", "(lfsr_noise(LFSR_17BIT, ((63920.0*2.0*exp(-x*3.5) + 15699.0*0.5) / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 131071.0)) * (1.0 + 0.5*abs(MOD_A)) ) * exp(-x * (1.2 + 3.0*abs(MOD_B))) + lfsr_noise(LFSR_4BIT, ((15699.0*exp(-x*2.5) + 63920.0*0.1) / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 15.0)) * (1.0 + RAND_OFFSET*0.2) ) * 0.3 * exp(-x * (1.8 + 2.5*abs(MOD_B)))) * 0.85" },
        // Rate of noise decays with x. MOD_A: Initial rate factor. MOD_B: Decay speed factor.

    /*191*/ { "POKEY \"Multi-Channel\" (Mixed)", "0.25 * (lfsr_noise(LFSR_4BIT, (63920.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 15.0)) * (1.0 + RAND_OFFSET*0.1)) + lfsr_noise(LFSR_5BIT, (15699.0 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 31.0)) * (1.0 + RAND_OFFSET*0.1)) + (x < (PI * (1.0 + 0.9*MOD_A)) ? 0.9 : -0.9) + lfsr_noise(LFSR_9BIT, (63920.0*0.5 / ( (FREQUENCY > 1.0 ? FREQUENCY : 1.0) * 511.0)) * (1.0 + 1.5*abs(MOD_B)) ) )" },
        // MOD_A: Tone PWM. MOD_B: Rate factor for 9-bit noise.

    // --- Authentic POKEY Noise (using lfsr_noise for simplicity, MOD_A=Vol, MOD_B=Fine Clock Adj) ---
    /*192*/ { "POKEY Pure 4b/64k",  "lfsr_noise(LFSR_4BIT, (63920.45 / ((FREQUENCY > 1.0 ? FREQUENCY : 1.0)*15.0)) * (1.0+0.2*MOD_B) ) * (0.5+0.49*MOD_A)" },
    /*193*/ { "POKEY Pure 17b/15k", "lfsr_noise(LFSR_17BIT, (15699.46 / ((FREQUENCY > 1.0 ? FREQUENCY : 1.0)*131071.0)) * (1.0+0.2*MOD_B) ) * (0.5+0.49*MOD_A)" },

    // --- Authentic POKEY "Tone AND Noise" (using lfsr_val, MOD_A=Vol, MOD_B=unused or for tone mod) ---
    // AUDCn = 001xxxxx -> Tone AND 17-bit Noise. Assume 17-bit noise clocked by 64kHz.
    /*194*/ { "POKEY T+N 17b/64k", "( (x<PI?1.0:0.0) && lfsr_val(LFSR_17BIT, (x/TWO_PI)*(63920.45 / (FREQUENCY > 1.0 ? FREQUENCY : 1.0)), 0.1*RAND_OFFSET) ? 1.0 : -1.0) * (0.5+0.49*MOD_A)"},

    // AUDCn = 000xxxxx -> Tone AND 4-bit Noise. Assume 4-bit noise clocked by 15kHz.
    /*195*/ { "POKEY T+N 4b/15k",  "( (x<PI?1.0:0.0) && lfsr_val(LFSR_4BIT, (x/TWO_PI)*(15699.46 / (FREQUENCY > 1.0 ? FREQUENCY : 1.0)), 0.1*RAND_OFFSET) ? 1.0 : -1.0) * (0.5+0.49*MOD_A)"},

    /*196*/ { "POKEY Poly17 FreeRun", "( (x < PI ? 1.0 : -1.0) * lfsr_val(LFSR_17BIT, 0.0, 0.0) ) * 0.7" },

    /*197*/ { "Evolving Metallic Bell", "sigma(k, 1.0, 6.0, 1.0, sin(x * k * (1.0 + k*0.14159 + MOD_A*0.1*k)) * exp(-x * k * (0.3 + MOD_B*0.7)) / k) * (1.0 + 0.3*sin(x*0.5 + RAND_OFFSET*PI)) * 0.4" },
    /*198*/ { "LFSR Granular Texture", "sin(x + lfsr_noise(LFSR_12BIT, 8.0 + MOD_A*16.0) * (0.5 + MOD_B*1.5)) * lfsr_clock(LFSR_8BIT, 0.3 + MOD_A*0.4) + lfsr_val(LFSR_16BIT, x/TWO_PI + RAND_OFFSET, MOD_B*0.5)*0.3 - 0.15" },
    /*199*/ { "Morphing Harmonics", "sigma(k, 1.0, 12.0, 1.0, sin(x*k + sin(x*k*0.25)*MOD_A*k*0.1) * (MOD_B > 0 ? (1.0/(k + abs(MOD_B)*8.0)) : exp(-k*abs(MOD_B)*0.5)) * (1.0 + 0.2*sin(x*k*3.0 + RAND_OFFSET*k))) * 0.15" },
    /*200*/ { "Breathing Pad", "(sin(x) + sin(x*1.498)*0.7 + sin(x*2.006)*0.5) * (0.6 + 0.4*sin(x*0.125 + MOD_A*PI)) * (1.0 + MOD_B*0.3*sin(x*0.0625 + RAND_OFFSET*TWO_PI)) + lfsr_noise(LFSR_4BIT, 0.5)*0.05" },
    /*201*/ { "Chaotic Oscillator", "tanh((sin(x + sin(x*1.618 + MOD_A*PI)*0.5) + sin(x*0.618 + MOD_B*PI_OVER_2)*0.7) * (2.0 + MOD_A*3.0)) * (0.8 + 0.2*lfsr_val(LFSR_7BIT, x/TWO_PI*3.0, RAND_OFFSET))" },
    /*202*/ { "Vocal Formant Morph", "sigma(k, 1.0, 16.0, 1.0, sin(x*k) * ((1.0/(1.0 + pow((k - (2.5 + MOD_A*4.0))/0.8, 2.0))) + (0.6/(1.0 + pow((k - (6.0 + MOD_B*6.0))/1.2, 2.0))) + (0.4/(1.0 + pow((k - 14.0)/2.0, 2.0)))) / sqrt(k)) * 0.2" },
    /*203*/ { "Glitchy Percussion", "lfsr_clock(LFSR_8BIT, 0.8) * sin(x*(1.0 + lfsr_noise(LFSR_12BIT, 32.0)*2.0) + lfsr_val(LFSR_16BIT, x/PI, MOD_A)*PI) * exp(-x*(2.0 + MOD_B*8.0)) + lfsr_clock(LFSR_4BIT, 0.5)*0.3*exp(-x*12.0)" },
    /*204*/ { "Phase Distortion Wave", "sin(x + sin(x*0.5 + MOD_A*PI)*MOD_B*4.0 + lfsr_noise(LFSR_6BIT, 1.0)*0.1) * (1.0 + 0.5*sin(x*0.25 + RAND_OFFSET*PI)) + tanh(sin(x*2.0 + MOD_A*PI_OVER_2)*3.0)*0.2*abs(MOD_B)" },
    /*205*/ { "Crystalline Arpeggio", "sigma(k, 1.0, 8.0, 1.0, sin(x*pow(2.0, floor(k + MOD_A*4.0) * 0.33333) + RAND_OFFSET*k*0.1) * exp(-x*k*(0.8 + MOD_B*1.2)) * lfsr_clock(LFSR_5BIT, 0.6 + k*0.05)) * 0.18" },
    /*206*/ { "Alien Communication", "lfsr_val(LFSR_11BIT, x/TWO_PI*2.0 + MOD_A, RAND_OFFSET)*2.0 - 1.0 + sin(x*(1.0 + lfsr_noise(LFSR_8BIT, 4.0)*0.5)) * (0.5 + 0.5*lfsr_clock(LFSR_7BIT, 0.4 + MOD_B*0.4)) + lfsr_noise(LFSR_4BIT, 16.0)*0.2" },

    /*207*/ { "FM: Classic EP (A=Tine B=Bell C=Ratio)", "sin(x + sin(x * 2.0) * (2.0 + 3.0 * MOD_A) * exp(-x*2.0) + sin(x * (3.0 + 2.0*MOD_C)) * (1.5 * MOD_B))" }, // A classic 2-Op electric piano sound that evolves. 
              // MOD_A: Controls the "tine" or sharp attack amount. MOD_B: Adds a 'bell-like' higher harmonic modulator. MOD_C: Changes the harmonic ratio of the bell component.
    /*208*/ { "FM: Growl Bass (A=Index B=Fdbk C=Ratio)", "sin(x + sin(x * (1.5 + MOD_C) + sin(x * (1.5 + MOD_C)) * MOD_B) * (1.0 + 4.0 * MOD_A))" }, // A 3-Op setup designed for aggressive bass sounds.
              // MOD_A: Main FM index (overall brightness/growl). MOD_B: Simulates feedback by modulating Op3 with itself, adding grit. MOD_C: Changes the modulator frequency for different growl characters.
    /*209*/ { "FM: Metallic Bell (A=Decay B=Index2 C=Ratio)", "sin(x + sin(x * (2.414 + MOD_C * 2.0) + sin(x * (5.75 - MOD_C * 3.0)) * (1.0 + 2.0*MOD_B)) * (4.0 * exp(-x * (1.5 + 4.0*MOD_A))))" }, // A classic inharmonic bell tone.
              // MOD_A: Controls the decay speed of the modulation, making it percussive. MOD_B: Controls the amount of the highest modulator (Op3). MOD_C: Sweeps the frequency ratio, creating shifting metallic tones.
    /*210*/ { "FM: Sci-Fi Drone (A=Evolve B=Chaos C=Pitch)", "sin(x * (1.0 + MOD_C * 0.01) + sin(x * 0.25) * (2.0 + 2.0*MOD_A) + sin(x * 13.0) * MOD_B)" }, // An evolving, complex pad/drone sound.
              // MOD_A: Slowly sweeps the main modulation index. MOD_B: Introduces a high-frequency, chaotic modulator. MOD_C: Modulates the pitch of the carrier for vibrato/instability.
    /*211*/ { "FM: Glitchy Noise (A=Index B=Bit C=Rate)", "sin(x + sin(x * (1.0 + 15.0*MOD_C) + floor(sin(x*27.0)* (8.0*MOD_B))/8.0) * (5.0*MOD_A))" }, // Uses FM to create harsh, digital noise effects.
              // MOD_A: The overall intensity of the FM effect. MOD_B: Simulates bitcrushing by using floor() on a modulator. MOD_C: Controls the frequency of the noisy modulator.
    };


/**
 * @brief Initialize the Polysonix wave system.
 * 
 * This function must be called once before using any other Polysonix wave functions.
 * It initializes:
 * - Random number generator seeding
 * - LFSR pre-computed tables
 * - Bytecode cache system
 * 
 * @return true on successful initialization, false on failure
 */
bool polysonix_wave_init(void) {
    printf("Initializing Polysonix Wave System...\n");
    
    // 1. Initialize random number generator with current time
    srand((unsigned int)time(NULL));
    printf("  Random number generator seeded with current time.\n");
    
    // 2. Initialize LFSR tables
    printf("  Initializing LFSR tables...\n");
    init_polysonix_lfsr_tables();
    
    // Verify LFSR initialization
    bool lfsr_success = true;
    for (int i = 0; i < NUM_LFSR_TYPES; i++) {
        if (!precomputed_lfsrs[i].initialized) {
            fprintf(stderr, "  ERROR: LFSR type %d failed to initialize.\n", i);
            lfsr_success = false;
        }
    }
    
    if (!lfsr_success) {
        fprintf(stderr, "ERROR: LFSR initialization failed.\n");
        return false;
    }
    
    // 3. Initialize bytecode cache
    printf("  Initializing bytecode cache...\n");
    initialize_bytecode_cache();
    
    if (!cache_initialized) {
        fprintf(stderr, "ERROR: Bytecode cache initialization failed.\n");
        return false;
    }
    
    // 4. Initialize default wave bytecode (optional - compile on demand instead)
    printf("  Default waves will be compiled on first use (lazy loading).\n");
    for (int i = 0; i < NUM_DEFAULT_WAVES; i++) {
        default_waves[i].compiled_bytecode = NULL; // Ensure they start as NULL
    }
    
    printf("Polysonix Wave System initialized successfully.\n");
    printf("  - %d LFSR types available\n", NUM_LFSR_TYPES);
    printf("  - %d default waveforms available\n", NUM_DEFAULT_WAVES);
    printf("  - Bytecode cache ready (size: %d entries)\n", CACHE_TABLE_SIZE);
    
    return true;
}

/**
 * @brief Deinitialize the Polysonix wave system.
 * 
 * This function should be called when shutting down the application.
 * It frees all allocated resources including:
 * - LFSR pre-computed tables
 * - Bytecode cache and all cached compiled expressions
 * - Default wave compiled bytecode
 */
void polysonix_wave_deinit(void) {
    printf("Deinitializing Polysonix Wave System...\n");
    
    // 1. Free default wave compiled bytecode
    printf("  Freeing default wave bytecode...\n");
    int freed_default_waves = 0;
    for (int i = 0; i < NUM_DEFAULT_WAVES; i++) {
        if (default_waves[i].compiled_bytecode != NULL) {
            free_bytecode_chunk(default_waves[i].compiled_bytecode);
            free(default_waves[i].compiled_bytecode);
            default_waves[i].compiled_bytecode = NULL;
            freed_default_waves++;
        }
    }
    printf("    Freed %d compiled default waves.\n", freed_default_waves);
    
    // 2. Free bytecode cache
    printf("  Freeing bytecode cache...\n");
    free_bytecode_cache();
    
    // 3. Free LFSR tables
    printf("  Freeing LFSR tables...\n");
    free_polysonix_lfsr_tables();
    
    printf("Polysonix Wave System deinitialized successfully.\n");
}

/**
 * @brief Get compiled bytecode for a default wave, compiling if necessary.
 * 
 * This function implements lazy loading - waves are compiled on first access
 * and cached for subsequent use.
 * 
 * @param wave_index Index of the default wave (0 to NUM_DEFAULT_WAVES-1)
 * @return Pointer to compiled bytecode, or NULL on error
 */
BytecodeChunk* get_default_wave_bytecode(int wave_index) {
    // Validate index
    if (wave_index < 0 || wave_index >= NUM_DEFAULT_WAVES) {
        fprintf(stderr, "Error: Invalid wave index %d (valid range: 0-%d)\n", 
                wave_index, NUM_DEFAULT_WAVES - 1);
        return NULL;
    }
    
    WaveDefinition* wave = &default_waves[wave_index];
    
    // If already compiled, return existing bytecode
    if (wave->compiled_bytecode != NULL) {
        return wave->compiled_bytecode;
    }
    
    // Compile the wave expression
    printf("Compiling default wave %d: '%s'\n", wave_index, wave->name);
    wave->compiled_bytecode = compile_expression_to_bytecode(wave->expression);
    
    if (wave->compiled_bytecode == NULL) {
        fprintf(stderr, "Error: Failed to compile default wave %d ('%s'): %s\n", 
                wave_index, wave->name, wave->expression);
        return NULL;
    }
    
    printf("  Successfully compiled wave %d (%d instructions)\n", 
           wave_index, count_bytecode_instructions(wave->compiled_bytecode));
    
    return wave->compiled_bytecode;
}

/**
 * @brief Get or compile bytecode for any expression with caching.
 * 
 * This function first checks the cache for pre-compiled bytecode.
 * If not found, it compiles the expression and caches the result.
 * 
 * @param expression Mathematical expression string
 * @return Pointer to compiled bytecode (cached), or NULL on error
 */
BytecodeChunk* get_or_compile_wave_bytecode(const char* expression) {
    if (!expression) {
        fprintf(stderr, "Error: NULL expression provided to get_or_compile_wave_bytecode\n");
        return NULL;
    }
    
    if (!cache_initialized) {
        fprintf(stderr, "Error: Bytecode cache not initialized. Call polysonix_wave_init() first.\n");
        return NULL;
    }
    
    // Check cache first
    BytecodeChunk* cached_chunk = lookup_cache(expression);
    if (cached_chunk != NULL) {
        // Cache hit - return existing compiled bytecode
        return cached_chunk;
    }
    
    // Cache miss - compile and cache the expression
    BytecodeChunk* new_chunk = compile_expression_to_bytecode(expression);
    if (new_chunk == NULL) {
        fprintf(stderr, "Error: Failed to compile expression: %s\n", expression);
        return NULL;
    }
    
    // Insert into cache (cache takes ownership)
    bool cache_success = insert_cache(expression, new_chunk);
    if (!cache_success) {
        fprintf(stderr, "Warning: Failed to cache compiled expression (continuing anyway): %s\n", expression);
        // Return the chunk anyway, caller is responsible for freeing it
        return new_chunk;
    }
    
    // Return the cached chunk
    return new_chunk;
}

/**
 * @brief Check if the Polysonix wave system is properly initialized.
 * 
 * @return true if system is ready for use, false otherwise
 */
bool polysonix_wave_is_initialized(void) {
    // Check LFSR initialization
    for (int i = 0; i < NUM_LFSR_TYPES; i++) {
        if (!precomputed_lfsrs[i].initialized) {
            return false;
        }
    }
    
    // Check cache initialization
    if (!cache_initialized) {
        return false;
    }
    
    return true;
}

/**
 * @brief Get system status and statistics.
 * 
 * @param stats Pointer to structure to fill with statistics (can be NULL)
 */
typedef struct {
    bool system_initialized;
    int lfsr_types_initialized;
    bool cache_initialized;
    size_t cache_entry_count;
    int default_waves_compiled;
    size_t total_memory_usage_estimate; // Rough estimate in bytes
} PolysonixWaveStats;

void polysonix_wave_get_stats(PolysonixWaveStats* stats) {
    if (!stats) return;
    
    memset(stats, 0, sizeof(PolysonixWaveStats));
    
    // Check LFSR initialization
    for (int i = 0; i < NUM_LFSR_TYPES; i++) {
        if (precomputed_lfsrs[i].initialized) {
            stats->lfsr_types_initialized++;
        }
    }
    
    // Cache stats
    stats->cache_initialized = cache_initialized;
    if (cache_initialized) {
        stats->cache_entry_count = bytecode_cache.count;
    }
    
    // Default waves compiled
    for (int i = 0; i < NUM_DEFAULT_WAVES; i++) {
        if (default_waves[i].compiled_bytecode != NULL) {
            stats->default_waves_compiled++;
        }
    }
    
    // System status
    stats->system_initialized = polysonix_wave_is_initialized();
    
    // Memory usage estimate (rough)
    stats->total_memory_usage_estimate = 0;
    
    // LFSR tables memory
    for (int i = 0; i < NUM_LFSR_TYPES; i++) {
        if (precomputed_lfsrs[i].initialized && precomputed_lfsrs[i].bit_table && precomputed_lfsrs[i].period > 0) { // Added period > 0 check
            stats->total_memory_usage_estimate += LFSR_TABLE_BYTES(precomputed_lfsrs[i].period); // Corrected: use .period
        }
    }
    
    // Cache memory (rough estimate: assume average 100 bytes per cached expression string + chunk overhead)
    // This is very rough. BytecodeChunk itself is large.
    stats->total_memory_usage_estimate += stats->cache_entry_count * (sizeof(CacheEntry) + 100 + sizeof(BytecodeChunk) + MAX_BYTECODE_SIZE/4); // String, data, struct
    
    // Default waves memory (rough estimate)
    stats->total_memory_usage_estimate += stats->default_waves_compiled * (sizeof(BytecodeChunk) + MAX_BYTECODE_SIZE/4); // Struct + average bytecode
}

/**
 * @brief Print system status and statistics to stdout.
 */
void polysonix_wave_print_stats(void) {
    PolysonixWaveStats stats;
    polysonix_wave_get_stats(&stats);
    
    printf("=== Polysonix Wave System Status ===\n");
    printf("System Initialized: %s\n", stats.system_initialized ? "YES" : "NO");
    printf("LFSR Types Ready: %d/%d\n", stats.lfsr_types_initialized, NUM_LFSR_TYPES);
    printf("Cache Initialized: %s\n", stats.cache_initialized ? "YES" : "NO");
    printf("Cache Entries: %zu\n", stats.cache_entry_count);
    printf("Default Waves Compiled: %d/%d\n", stats.default_waves_compiled, NUM_DEFAULT_WAVES);
    printf("Estimated Memory Usage: %zu bytes (%.1f KB)\n", stats.total_memory_usage_estimate, stats.total_memory_usage_estimate / 1024.0f);
    printf("=====================================\n");
}

/* Usage Example:
int main() {
    // Initialize the system
    if (!polysonix_wave_init()) {
        fprintf(stderr, "Failed to initialize Polysonix wave system!\n");
        return 1;
    }
    
    // Use the system...
    BytecodeChunk* wave0 = get_default_wave_bytecode(0);
    BytecodeChunk* custom = get_or_compile_wave_bytecode("sin(x) * 0.5");
    
    // Print stats
    polysonix_wave_print_stats();
    
    // Cleanup when done
    polysonix_wave_deinit();
    
    return 0;
}
*/


/*
*********** WAVE SEQUENCING STATE BITS
Bit 0: SEQ_END (End of Sequence)
Purpose: Marks the final step of the sequence to be played.
Behavior: After the specified Cycle Count for this step completes (and considering SEQ_SUSTAIN), the sequence stops advancing. The channel might continue sounding based on the ADSR envelope's release phase, but it will hold the waveform from this final step (or silence if combined with SEQ_MUTE). This bit typically overrides SEQ_LOOP.
Bit 1: SEQ_LOOP (Jump to Start)
Purpose: Creates simple loops by returning to the beginning of the sequence.
Behavior: After the specified Cycle Count for this step completes (and considering SEQ_SUSTAIN), the sequence step index is reset to 0, and the sequence continues playing from the first step. Ignored if SEQ_END is also set on the same step.
Bit 2: SEQ_SUSTAIN (Sustain Point / Gate)
Purpose: Holds the sequence at this step as long as the note is held (gated by Note On/Off). Essential for mimicking ADSR sustain behavior within the sequence.
Behavior: Play the specified Cycle Count for this step. Then:
If Note is ON (from AUDCTLB): Repeat the waveform from this step indefinitely (effectively ignoring the cycle count from now on for this note).
If Note goes OFF: Immediately stop sustaining and proceed to evaluate the next step in the sequence (advancing the step counter). This allows the sequence to potentially play different "release" waveforms after the sustain point.
Bit 3: SEQ_MUTE (Silence Step)
Purpose: Creates rests or silences within the sequence.
Behavior: During this step, the channel's waveform output is forced to zero (silence). However, the Cycle Count is still respected, and the sequence timing advances normally. This allows inserting timed gaps.
Bit 4: SEQ_RESET_LFO (Reset Associated LFO)
Purpose: Synchronizes LFO modulation to specific points in the sequence.
Behavior: When the sequence enters this step (before playing its cycles), reset the phase of the LFO associated with this channel's pair (LFO 0/1 for group 0, LFO 2/3 for group 1, etc.) back to 0.0.
Bit 5: SEQ_RETRIGGER_ADSR (Retrigger Envelope)
Purpose: Creates percussive effects or accents within a held note.
Behavior: When the sequence enters this step, force the ADSR envelope associated with this channel's pair back into its Attack phase (Phase 1), restarting the envelope as if a new note-on occurred (even if the physical note is still held).
Bit 6: WSTA_SEQ_PITCH_SCALE (Scale Pitch by Wave Length) - NEW
Purpose: Adjusts the playback pitch based on the selected waveform's length relative to the maximum length (256). Shorter waves play back proportionally faster (higher pitch).
Behavior:
If this bit is OFF (0): The waveform lookup uses the channel's normal phase advancement (chan->phase), scaled by the step's wave_length. The pitch is determined solely by the channel's frequency settings. (Original behavior).
If this bit is ON (1): The phase used for waveform lookup is multiplied by a factor determined by the wave_length.
wave_length 256: Factor = 1.0 (No change)
wave_length 128: Factor = 2.0 (Plays 1 octave higher)
wave_length 64: Factor = 4.0 (Plays 2 octaves higher)
wave_length 32: Factor = 8.0 (Plays 3 octaves higher)
The lookup becomes effectively: effective_phase = fmodf(chan->phase * pitch_scale_factor, 1.0f); sample_index = (uint16_t)(effective_phase * (float)wave_length);
Bit 7: RESERVED_2
*/
#define WSTA_SEQ_END             0x01
#define WSTA_SEQ_LOOP            0x02
#define WSTA_SEQ_SUSTAIN         0x04
#define WSTA_SEQ_MUTE            0x08
#define WSTA_SEQ_RESET_LFO       0x10
#define WSTA_SEQ_RETRIGGER_ADSR  0x20
#define WSTA_SEQ_PITCH_SCALE     0x40
#define WSTA_SEQ_BLEND_PREVIOUS  0x80


/**
 * @brief Retrieves the original mathematical expression string for a default wave.
 *
 * Looks up the expression string associated with a specific wave index
 * in the global `default_waves` array.
 *
 * @param wave_index The index (0 to NUM_DEFAULT_WAVES - 1) of the desired wave.
 * @return const char* Pointer to the expression string if the index is valid,
 *         otherwise returns NULL. The returned string is owned by the
 *         `default_waves` array and should not be modified or freed by the caller.
 */
const char* get_wave_expression_string(int wave_index) {
    if (wave_index < 0 || wave_index >= NUM_DEFAULT_WAVES) { fprintf(stderr, "Error (get_wave_expression_string): Invalid wave index %d (valid range 0-%d).\n", wave_index, NUM_DEFAULT_WAVES - 1); return NULL; }
    return default_waves[wave_index].expression;
}


uint8_t wave_sequences[164][96] = {
    // Sequence 0: Simple Square Wave Tone
    {
        4, 1, WSTA_SEQ_SUSTAIN | WSTA_SEQ_END,  // Step 0: Square, 1 cycle, sustain and end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 1: Alternating Triangle and Square
    {
        2, 1, 0,              // Step 0: Triangle, 1 cycle, no flags
        4, 1, WSTA_SEQ_LOOP,  // Step 1: Square, 1 cycle, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END
    },
    // Sequence 2: Noise Hit
    {
        39, 1, WSTA_SEQ_END,  // Step 0: Noise, 1 cycle, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 3: Rising Pitch Effect
    {
        0,  1, WSTA_SEQ_PITCH_SCALE,                  // Step 0: Sine, 1 cycle, pitch scale
        8,  1, WSTA_SEQ_PITCH_SCALE,                  // Step 1: Wave 8, 1 cycle, pitch scale
        16, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_END,   // Step 2: Wave 16, 1 cycle, pitch scale and end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END
    },
    // Sequence 4: Rhythmic Pattern
    {
        4, 1, 0,                           // Step 0: Square, 1 cycle, no flags
        4, 1, WSTA_SEQ_MUTE,               // Step 1: Square, 1 cycle, mute
        4, 1, 0,                           // Step 2: Square, 1 cycle, no flags
        4, 1, WSTA_SEQ_MUTE | WSTA_SEQ_LOOP, // Step 3: Square, 1 cycle, mute and loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 5: Staccato Effect
    {
        4, 4, WSTA_SEQ_RETRIGGER_ADSR,              // Step 0: Square, 4 cycles, retrigger ADSR
        4, 4, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_LOOP, // Step 1: Square, 4 cycles, retrigger ADSR and loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END
    },
    // Sequence 6: LFO Reset
    {
        4, 1, WSTA_SEQ_RESET_LFO,  // Step 0: Square, 1 cycle, reset LFO
        4, 1, WSTA_SEQ_LOOP,       // Step 1: Square, 1 cycle, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END
    },
    // Sequence 7: Waveform Cycle
    {
        2, 1, 0,              // Step 0: Triangle, 1 cycle, no flags
        6, 1, 0,              // Step 1: Sawtooth, 1 cycle, no flags
        4, 1, WSTA_SEQ_LOOP,  // Step 2: Square, 1 cycle, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
        // Sequence 8: Morphing Sequence
    // Cycles through sine, triangle, square, sawtooth to create a morphing effect
    {
        0, 4, 0,              // Step 0: Sine, 4 cycles
        2, 4, 0,              // Step 1: Triangle, 4 cycles
        4, 4, 0,              // Step 2: Square, 4 cycles
        6, 4, WSTA_SEQ_LOOP,  // Step 3: Sawtooth, 4 cycles, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 9: Rhythmic Pattern
    // Alternates square and triangle with mutes for a rhythmic beat
    {
        4, 1, 0,                           // Step 0: Square, 1 cycle
        4, 1, WSTA_SEQ_MUTE,               // Step 1: Mute, 1 cycle
        2, 2, 0,                           // Step 2: Triangle, 2 cycles
        2, 1, WSTA_SEQ_MUTE | WSTA_SEQ_LOOP, // Step 3: Mute, 1 cycle, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 10: Sustain and Release
    // Builds up to a sustain on square, releases with sawtooth and noise
    {
        0, 2, 0,              // Step 0: Sine, 2 cycles
        2, 2, 0,              // Step 1: Triangle, 2 cycles
        4, 1, WSTA_SEQ_SUSTAIN, // Step 2: Square, 1 cycle, sustain
        6, 2, 0,              // Step 3: Sawtooth, 2 cycles
        39, 1, WSTA_SEQ_END,  // Step 4: Noise, 1 cycle, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 11: LFO Sync
    // Resets LFO periodically for synced modulation effects
    {
        4, 4, WSTA_SEQ_RESET_LFO,  // Step 0: Square, 4 cycles, reset LFO
        4, 4, 0,                   // Step 1: Square, 4 cycles
        4, 4, WSTA_SEQ_RESET_LFO | WSTA_SEQ_LOOP, // Step 2: Square, 4 cycles, reset LFO, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 12: Pitch Scaling
    // Uses waveforms with lengths 256, 128, 64, 32 for rising pitch
    {
        0,  1, WSTA_SEQ_PITCH_SCALE,                  // Step 0: Wave 0 (256), 1 cycle, pitch scale
        16, 1, WSTA_SEQ_PITCH_SCALE,                  // Step 1: Wave 16 (128), 1 cycle, pitch scale
        32, 1, WSTA_SEQ_PITCH_SCALE,                  // Step 2: Wave 32 (64), 1 cycle, pitch scale
        48, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_END,   // Step 3: Wave 48 (32), 1 cycle, pitch scale, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 13: Percussive Sequence
    // Retriggers ADSR for a stuttering percussive effect
    {
        4, 1, WSTA_SEQ_RETRIGGER_ADSR,              // Step 0: Square, 1 cycle, retrigger ADSR
        4, 1, 0,                                    // Step 1: Square, 1 cycle
        4, 1, WSTA_SEQ_RETRIGGER_ADSR,              // Step 2: Square, 1 cycle, retrigger ADSR
        4, 1, WSTA_SEQ_LOOP,                        // Step 3: Square, 1 cycle, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 14: Complex Loop
    // Loops through multiple waveforms with varying durations
    {
        0,  2, 0,              // Step 0: Sine, 2 cycles
        2,  3, 0,              // Step 1: Triangle, 3 cycles
        4,  1, 0,              // Step 2: Square, 1 cycle
        6,  2, 0,              // Step 3: Sawtooth, 2 cycles
        39, 1, WSTA_SEQ_LOOP,  // Step 4: Noise, 1 cycle, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 15: Muted Steps
    // Uses mutes to create a rhythmic sequence with rests
    {
        2,  1, 0,                           // Step 0: Triangle, 1 cycle
        4,  1, WSTA_SEQ_MUTE,               // Step 1: Mute, 1 cycle
        6,  2, 0,                           // Step 2: Sawtooth, 2 cycles
        39, 1, WSTA_SEQ_MUTE | WSTA_SEQ_LOOP, // Step 3: Mute, 1 cycle, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 16: Space Invaders - Descending Tones
    // Square wave with pitch scaling for that classic invader march
    {
        4, 1, WSTA_SEQ_PITCH_SCALE,  // High pitch square
        4, 1, WSTA_SEQ_PITCH_SCALE,  // Slightly lower
        4, 1, WSTA_SEQ_PITCH_SCALE,  // Lower still
        4, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP,  // Lowest, then loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 17: Cybernetic Heartbeat Glitch
    // Concept: A rhythmic pulse that breaks down into glitches.
    {
        4,  1, WSTA_SEQ_RETRIGGER_ADSR, // Low Square thump (Wave 4, Size 256)
        0,  1, WSTA_SEQ_MUTE,           // Silence (Using wave 0, cycles don't matter when muted)
        4,  1, WSTA_SEQ_RETRIGGER_ADSR, // Low Square thump
        0,  1, WSTA_SEQ_MUTE,           // Silence
        163,1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Glitch Sine High (Wave 35, Size 64 -> Index 128+35=163)
        38, 1, WSTA_SEQ_RETRIGGER_ADSR, // Pulse Train Wreck quick burst (Wave 38, Size 256)
        4,  1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_LOOP, // Low Square thump & Loop back to start
        0,  0, WSTA_SEQ_END, // End marker (sequence loops before reaching here)
        // Padding
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 18: Galaxian - Laser Shot
    // Fast descending sawtooth into noise for a zap
    {
        6, 1, WSTA_SEQ_PITCH_SCALE,  // High sawtooth
        6, 1, WSTA_SEQ_PITCH_SCALE,  // Lower sawtooth
        39, 1, WSTA_SEQ_END,         // Noise burst, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 19: Donkey Kong - Jump Sound
    // Quick ascending square wave for the leap
    {
        4, 1, WSTA_SEQ_PITCH_SCALE,  // Low pitch
        4, 1, WSTA_SEQ_PITCH_SCALE,  // Higher pitch
        4, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_END,  // Highest, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 20: Mario Bros - Coin Collect
    // Two rising square tones for that coin grab
    {
        4, 1, WSTA_SEQ_PITCH_SCALE,  // Low tone
        4, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_END,  // High tone, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 21: Q*Bert - Hop Sound
    // Short square wave hops with mutes
    {
        4, 1, 0,              // Hop tone
        4, 1, WSTA_SEQ_MUTE,  // Silence
        4, 1, 0,              // Hop tone
        4, 1, WSTA_SEQ_MUTE | WSTA_SEQ_END,  // Silence, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 22: Galaxian - Explosion
    // Noise with descending pitch for a boom
    {
        39, 1, WSTA_SEQ_PITCH_SCALE,  // High noise
        39, 1, WSTA_SEQ_PITCH_SCALE,  // Lower noise
        39, 1, WSTA_SEQ_PITCH_SCALE,  // Lower still
        39, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_END,  // Lowest, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 23: Space Invaders - UFO
    // Looping square wave siren
    {
        4, 1, WSTA_SEQ_PITCH_SCALE,  // High pitch
        4, 1, WSTA_SEQ_PITCH_SCALE,  // Low pitch
        4, 1, WSTA_SEQ_PITCH_SCALE,  // Mid pitch
        4, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP,  // Back to high, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
        // Sequence 24: Clipped Sine Hell
    // Looping clipped sine waves with retriggered ADSR for a constant, distorted pulse
    {
        18, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_LOOP,  // Clipped sine, retrigger ADSR, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 25: Overfolded Saw Shred
    // Overfolded sawtooth with pitch scaling and mutes for a jagged, metallic grind
    {
        36, 2, WSTA_SEQ_PITCH_SCALE,           // Overfolded saw, pitch scaled
        36, 1, WSTA_SEQ_MUTE,                  // Mute for stutter
        36, 2, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP,  // Overfolded saw, pitch scaled, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 26: Glitch Noise Storm
    // Glitchy sine and noise with LFO resets for chaotic modulation
    {
        35, 1, WSTA_SEQ_RESET_LFO,           // Glitch sine, reset LFO
        39, 1, 0,                            // Noise
        35, 1, WSTA_SEQ_RESET_LFO,           // Glitch sine, reset LFO
        39, 1, WSTA_SEQ_LOOP,                // Noise, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 27: Pulse Train Mayhem
    // Short, sharp pulses with varying amplitudes and mutes, looped for a broken-machine effect
    {
        38, 1, 0,                            // Pulse train
        38, 1, WSTA_SEQ_MUTE,                // Mute
        38, 1, 0,                            // Pulse train
        38, 1, WSTA_SEQ_MUTE | WSTA_SEQ_LOOP,  // Mute, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 28: Harmonic Noise Blast Loop
    // Stacked high-frequency sines, clipped and looped with pitch scaling for dissonance
    {
        39, 1, WSTA_SEQ_PITCH_SCALE,           // Harmonic noise blast, pitch scaled
        39, 1, WSTA_SEQ_PITCH_SCALE,           // Again, pitch scaled
        39, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP,  // Pitch scaled, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 29: Shredded Saw Chaos
    // Shredded saws with extreme clipping, retriggered ADSR, and looping
    {
        33, 1, WSTA_SEQ_RETRIGGER_ADSR,           // Shredded saw, retrigger ADSR
        33, 1, 0,                                 // Shredded saw
        33, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_LOOP,  // Shredded saw, retrigger ADSR, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 30: Bit-Crushed Square Hell
    // Quantized square waves with mutes and looping for a lo-fi digital meltdown
    {
        34, 1, 0,                            // Bit-crushed square
        34, 1, WSTA_SEQ_MUTE,                // Mute
        34, 1, 0,                            // Bit-crushed square
        34, 1, WSTA_SEQ_MUTE | WSTA_SEQ_LOOP,  // Mute, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 31: Razor Pulse Rampage
    // Ultra-narrow pulses with saw-like tails, pitch scaling, and LFO resets, looped
    {
        32, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_RESET_LFO,  // Razor pulse, pitch scaled, reset LFO
        32, 1, WSTA_SEQ_PITCH_SCALE,                       // Razor pulse, pitch scaled
        32, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP,       // Razor pulse, pitch scaled, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
        // Sequence 32: Kick Drum
    // A deep sine wave with a quick pitch drop for a classic kick sound
    {
        0, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_RETRIGGER_ADSR,  // Sine, pitch scaled, retrigger ADSR
        0, 1, WSTA_SEQ_PITCH_SCALE,                            // Sine, pitch scaled (lower)
        0, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_END,             // Sine, pitch scaled (lowest), end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 33: Snare Drum
    // A sharp noise burst followed by a muted tone for a snare-like snap
    {
        39, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Noise, retrigger ADSR
        4,  1, WSTA_SEQ_MUTE | WSTA_SEQ_END,  // Square, muted, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 34: Hi-Hat
    // Short, high-pitched noise for a crisp hi-hat sound
    {
        39, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_END,  // Noise, high pitch, retrigger ADSR, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 35: Pitched Tom (Uses Triangle)
    {
        2,  1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // High Tri (Size 0)
        130,1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Mid Tri (Size 2, index 2)
        130,1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Mid Tri
        2,  2, WSTA_SEQ_END,                                   // Low Tri decay (Size 0)
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 36: Clap
    // Layered noise with a slight delay for a clap effect
    {
        39, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Noise, retrigger ADSR
        39, 1, WSTA_SEQ_MUTE,            // Mute for delay
        39, 1, WSTA_SEQ_END,             // Noise again, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 37: Resonating Metal Impact
    // Sharp metallic hit followed by evolving resonance using different wave sizes.
    {
        55, 1, WSTA_SEQ_RETRIGGER_ADSR, // Metal Impact (Wave 55, Size 256)
        53, 1, WSTA_SEQ_PITCH_SCALE,    // Cymbal short burst, high pitch (Wave 53, Size 256)
        146,1, WSTA_SEQ_PITCH_SCALE,    // Clipped Sine burst, mid pitch (Wave 18, Size 128 -> Index 64+18=82, ERROR, used 128+18=146)
                                        // Corrected: Wave 18, Size 128 -> Index 64+18 = 82
                                        // Let's use Wave 18 Size 64 -> Index 128+18 = 146 instead. Pitched lower.
        128,8, WSTA_SEQ_SUSTAIN,        // Sine resonance sustain (Wave 0, Size 128 -> Index 64+0=64, ERROR, used 128)
                                        // Corrected: Wave 0, Size 128 -> Index 64+0=64. Let's use 128 (Size 64 index 0)
        192,4, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_END, // Sine shimmer release, high pitch (Wave 0, Size 32 -> Index 192+0=192)
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 38: Cowbell
    // Concept: Sharper attack using a metallic wave, followed by a pitched square tone and a brief ring-out.
    // Timing (@1kHz): 1ms attack + 4ms tone + 6ms ring = 11ms + envelope release.
    {
        55, 1, WSTA_SEQ_RETRIGGER_ADSR, // Sharp Metal Impact (Wave 55) for attack
        4,  4, WSTA_SEQ_PITCH_SCALE,    // Square wave (Wave 4) pitched up for the main tone
        61, 6, 0,                       // Metallic Percussion (Wave 61) for a short ring/decay
        4,  1, WSTA_SEQ_END,            // Short Square click to finish cleanly
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 39: Bass Drum
    // Concept: Adds a noise tick for attack definition and uses different sine sizes for pitch drop and body.
    // Timing (@1kHz): 1ms tick + 1ms high sine + 3ms mid sine + 12ms low sine = 17ms + envelope release.
    {
        51, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Hi-Hat tick (Wave 51) very high pitch for click
        192,1, WSTA_SEQ_PITCH_SCALE,    // High Sine pitch (Wave 0, Size 32 -> 192)
        64, 3, WSTA_SEQ_PITCH_SCALE,    // Mid Sine pitch (Wave 0, Size 128 -> 64) slightly longer
        0, 12, WSTA_SEQ_END,            // Low Sine body/decay (Wave 0, Size 256)
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
        // Sequence 40: Gated Noise Sweep
    {
        79, 1, WSTA_SEQ_PITCH_SCALE, // Noise Rise (Size 1, Index 15)
        39, 1, WSTA_SEQ_PITCH_SCALE, // Noise Blast (Size 0) - Higher pitch start
        39, 1, WSTA_SEQ_MUTE,
        39, 1, WSTA_SEQ_RETRIGGER_ADSR,
        39, 1, WSTA_SEQ_MUTE,
        39, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_LOOP,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 41: Complex Perc Hit (Wood + Metal + Sine)
    {
        40, 1, WSTA_SEQ_RETRIGGER_ADSR, // Wood (Size 0)
        55, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Metal High (Size 0)
        192,2, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Sine Body High (Size 3)
        0,  3, WSTA_SEQ_END,             // Sine Body Low Decay (Size 0)
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 42: Chimes
    // A bright, ringing triangle wave with sustain to let it ring out
    {
        2, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_SUSTAIN,  // Triangle, retrigger ADSR, sustain
        2, 1, WSTA_SEQ_END,                                // Triangle, end (to allow sustain)
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 43: Timbale -> Short Glitch Burst
    {
        35, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Glitch Sine High
        38, 1, WSTA_SEQ_RETRIGGER_ADSR, // Pulse Train Wreck
        39, 1, WSTA_SEQ_MUTE,
        35, 1, WSTA_SEQ_END, // Glitch Sine Low end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 44: Minipops7-style Sound 1
    // A quirky electronic percussion sound with noise and square wave
    {
        39, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_RESET_LFO,  // Noise, retrigger ADSR, reset LFO
        4,  1, WSTA_SEQ_END,                                  // Square, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 45: Minipops7-style Sound 2
    // Another electronic percussion sound with pitched square and noise
    {
        4,  1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_RESET_LFO,  // Square, pitched, retrigger ADSR, reset LFO
        39, 1, WSTA_SEQ_END,                                  // Noise, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 46: Alien Computer
    // A very short, high-pitched square wave for a tick sound
    {
        28, 2, WSTA_SEQ_PITCH_SCALE, // Pixel Crunch High
        28, 2, 0,                    // Pixel Crunch Mid
        31, 2, WSTA_SEQ_RESET_LFO,   // Hyperspace Glitch LFO
        28, 2, WSTA_SEQ_LOOP,        // Pixel Crunch Loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 47: Pops
    // Concept: Sharp, high-pitched noise attack, brief harsh body, then cuts off.
    // Timing (@1kHz): 1ms noise + 1ms body = 2ms + envelope release (very short).
    {
        103,1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Noise Blast (Wave 39, Size 128 -> 103) High pitched start
        227,1, WSTA_SEQ_PITCH_SCALE,    // Glitch Sine (Wave 35, Size 32 -> 227) very short harsh body, pitched
        0,  1, WSTA_SEQ_MUTE | WSTA_SEQ_END, // Mute immediately after body
        0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
        // Sequence 48: Evolving Pad
    // A lush, evolving pad with sine, triangle, and sawtooth waves
    {
        0, 4, 0,              // Step 0: Sine, 4 cycles
        2, 4, 0,              // Step 1: Triangle, 4 cycles
        6, 1, WSTA_SEQ_SUSTAIN, // Step 2: Sawtooth, 1 cycle, sustain
        0, 4, WSTA_SEQ_END,   // Step 3: Sine, 4 cycles, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 49: Bright Lead
    // A cutting lead with sawtooth waves and a slight detune effect
    {
        6, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Step 0: Sawtooth, retrigger ADSR
        7, 1, WSTA_SEQ_LOOP,            // Step 1: Slightly detuned sawtooth, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 50: Punchy Bass
    // A deep bass with a quick pitch bend at the start
    {
        4, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_RETRIGGER_ADSR,  // Step 0: Square, pitch bend, retrigger ADSR
        4, 1, WSTA_SEQ_END,                                    // Step 1: Square, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 51: Crystal Bell
    // A ringing bell sound with sustain
    {
        0, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Step 0: Sine, retrigger ADSR
        2, 1, WSTA_SEQ_SUSTAIN,         // Step 1: Triangle, sustain
        0, 4, WSTA_SEQ_END,             // Step 2: Sine, 4 cycles, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 52: Synth Pluck
    // Concept: Brighter attack using Additive wave, slight pitch down during decay with Sine.
    // Timing (@1kHz): 2ms attack + 6ms decay = 8ms + envelope release.
    {
        58, 2, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Additive Synth (Wave 58) bright attack, pitched high
        0,  6, WSTA_SEQ_END,            // Sine (Wave 0) decay, slightly longer duration
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },

    // Sequence 53: Space FX
    // An experimental effect with noise and pitch scaling
    {
        39, 1, WSTA_SEQ_PITCH_SCALE,  // Step 0: Noise, pitch scaled
        39, 1, WSTA_SEQ_MUTE,         // Step 1: Mute
        39, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP,  // Step 2: Noise, pitch scaled, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 54: Analog Brass
    // A warm, brassy sound with sawtooth and square waves
    {
        6, 2, 0,              // Step 0: Sawtooth, 2 cycles
        4, 2, WSTA_SEQ_SUSTAIN, // Step 1: Square, 2 cycles, sustain
        6, 2, WSTA_SEQ_END,   // Step 2: Sawtooth, 2 cycles, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 55: Formant Sweep Pad
    // A bright, digital chime with sine and triangle waves
    {
        22, 4, WSTA_SEQ_PITCH_SCALE, // Formant wave, sweep up
        22, 4, WSTA_SEQ_PITCH_SCALE,
        22, 4, WSTA_SEQ_SUSTAIN,     // Sustain at higher pitch
        22, 4, 0,                    // Normal pitch on release
        22, 4, WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 56: Evolving Acid Bassline
    // Concept: Morphing timbre bassline using pitch scale for filter-like effects and LFO resets.
    {
        4,  2, WSTA_SEQ_RETRIGGER_ADSR, // Square (Wave 4)
        6,  2, WSTA_SEQ_RESET_LFO,      // Saw (Wave 6), LFO Reset
        4,  2, WSTA_SEQ_PITCH_SCALE,    // Square, Pitch Scale High (filter up)
        6,  2, WSTA_SEQ_RESET_LFO,      // Saw, LFO Reset
        32, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Razor Pulse (Wave 32), very high pitch accent
        18, 3, WSTA_SEQ_RESET_LFO,      // Clipped Sine (Wave 18), LFO Reset
        4,  2, 0,                       // Square
        6,  2, WSTA_SEQ_LOOP,           // Saw & Loop
        // Padding
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 57: Sweeping Pad
    // A pad with a sweeping filter effect using pitch scaling
    {
        6, 4, WSTA_SEQ_PITCH_SCALE,  // Step 0: Sawtooth, pitch scaled
        6, 4, WSTA_SEQ_PITCH_SCALE,  // Step 1: Sawtooth, pitch scaled (higher)
        6, 4, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_SUSTAIN,  // Step 2: Sawtooth, pitch scaled (highest), sustain
        6, 4, WSTA_SEQ_PITCH_SCALE,  // Step 3: Sawtooth, pitch scaled (lower)
        6, 4, WSTA_SEQ_END,          // Step 4: Sawtooth, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 58: Electric Piano
    // A percussive, bell-like sound with a quick decay
    {
        0, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Step 0: Sine, retrigger ADSR
        2, 1, WSTA_SEQ_END,             // Step 1: Triangle, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 59: Synth Strings
    // A smooth, string-like pad with sawtooth and triangle waves
    {
        6, 4, 0,              // Step 0: Sawtooth, 4 cycles
        2, 4, WSTA_SEQ_SUSTAIN, // Step 1: Triangle, 4 cycles, sustain
        6, 4, WSTA_SEQ_END,   // Step 2: Sawtooth, 4 cycles, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 60: Retro Arp
    // A fast, arpeggiated sequence with square and triangle waves
    {
        4, 1, 0,              // Step 0: Square, 1 cycle
        2, 1, 0,              // Step 1: Triangle, 1 cycle
        4, 1, 0,              // Step 2: Square, 1 cycle
        2, 1, WSTA_SEQ_LOOP,  // Step 3: Triangle, 1 cycle, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 61: Spectral Drone Morph
    // Concept: Slowly morphing complex harmonic waves with LFO modulation.
    {
        21, 8, WSTA_SEQ_RESET_LFO,      // Sine+Harmonics (Wave 21) LFO Start/Reset
        22, 8, 0,                       // Formant Wave (Wave 22)
        58, 8, WSTA_SEQ_RESET_LFO,      // Additive Synth (Wave 58) LFO Reset
        44, 8, WSTA_SEQ_SUSTAIN,        // Vocal Ah (Wave 44) Sustain Point
        22, 8, WSTA_SEQ_RESET_LFO,      // Formant Wave (Release Phase 1 with LFO)
        21, 8, WSTA_SEQ_END,            // Sine+Harmonics (Release Phase 2)
        // Padding
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },

    // Sequence 62: Harsh Wobble Bass
    // A bold, brassy sound with sawtooth and square waves
    {
        33, 2, WSTA_SEQ_RESET_LFO | WSTA_SEQ_RETRIGGER_ADSR, // Shredded Saw LFO
        36, 2, WSTA_SEQ_RESET_LFO | WSTA_SEQ_RETRIGGER_ADSR, // Overfolded Saw LFO
        33, 2, WSTA_SEQ_SUSTAIN,     // Sustain Shredded
        36, 2, WSTA_SEQ_END,         // Release Overfolded
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 63: Retro Lead
    // A classic lead sound with a sawtooth wave and vibrato
    {
        6, 1, WSTA_SEQ_RESET_LFO | WSTA_SEQ_LOOP,  // Step 0: Sawtooth, reset LFO, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
        // Sequence 64: Rough Industrial Pulse
    // A harsh, looping sequence with clipped sines and noise for an industrial feel
    {
        18, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Clipped sine, retrigger ADSR
        39, 1, 0,                        // Noise
        18, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Clipped sine, retrigger ADSR
        39, 1, WSTA_SEQ_LOOP,            // Noise, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 65: Splashy Waterfall
    // A sequence that mimics water with noise and pitch-scaled sines
    {
        39, 2, WSTA_SEQ_PITCH_SCALE,  // Noise, pitch scaled
        0,  2, WSTA_SEQ_PITCH_SCALE,  // Sine, pitch scaled
        39, 2, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP,  // Noise, pitch scaled, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 66: Reversed Envelope Pad
    // A pad that evolves from sine to sawtooth with sustain
    {
        8,  4, WSTA_SEQ_RETRIGGER_ADSR, // Saw/Sine start quietly
        14, 4, 0,                       // Tri/Sine builds
        21, 4, WSTA_SEQ_SUSTAIN,        // Sine+Harmonics sustain loud
        6,  4, WSTA_SEQ_END,            // Saw release quick fade
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 67: Earthy Percussion
    // A percussive sequence with wooden and natural tones
    {
        40, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Wooden percussion, retrigger ADSR
        42, 1, WSTA_SEQ_END,             // Chime, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 68: Granular Cloud
    // A bubbling effect with noise and pitch-scaled sines
    {
        51, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // High Hat Tick High
        51, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // High Hat Tick Mid
        46, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Water Droplet Low
        51, 1, WSTA_SEQ_MUTE,
        46, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_LOOP,        // Loop Droplet
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 69: Phase Mod FX
    // A glitchy, digital sequence with rapid waveform changes and LFO resets
    {
        27, 2, WSTA_SEQ_RESET_LFO,   // Ghost Wail LFO
        23, 2, WSTA_SEQ_PITCH_SCALE, // Sine*Saw pitch
        27, 2, WSTA_SEQ_RESET_LFO | WSTA_SEQ_SUSTAIN, // Sustain Wail
        23, 2, WSTA_SEQ_END,         // Release Sine*Saw
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 70: Pounding Bass
    // A heavy bass with pitch scaling and looping for rhythm
    {
        4, 1, WSTA_SEQ_PITCH_SCALE,  // Square, pitch scaled
        4, 1, WSTA_SEQ_PITCH_SCALE,  // Square, pitch scaled (lower)
        4, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP,  // Square, pitch scaled (lowest), loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 71: Additive Build Pad
    // A vocal-like pad with sine and triangle waves
    {
        0,  4, WSTA_SEQ_RETRIGGER_ADSR, // Sine Start
        21, 4, 0,                       // Sine + Harmonics
        58, 4, WSTA_SEQ_SUSTAIN,        // Additive Synth Sustain
        21, 4, WSTA_SEQ_END,            // Release Sine+Harmonics
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 72: Metallic Shimmer
    // A shimmering, metallic sound with high-frequency content
    {
        39, 1, WSTA_SEQ_PITCH_SCALE,  // Noise, pitch scaled
        18, 1, WSTA_SEQ_PITCH_SCALE,  // Clipped sine, pitch scaled
        39, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP,  // Noise, pitch scaled, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 73: Organic Groove
    // A rhythmic sequence with natural and percussive elements
    {
        40, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Wooden percussion, retrigger ADSR
        42, 1, 0,                        // Chime
        40, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Wooden percussion, retrigger ADSR
        42, 1, WSTA_SEQ_LOOP,            // Chime, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 74: Cybernetic Pulse
    // A pulsing, high-tech sequence with square and sawtooth waves
    {
        4, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Square, retrigger ADSR
        6, 1, 0,                        // Sawtooth
        4, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Square, retrigger ADSR
        6, 1, WSTA_SEQ_LOOP,            // Sawtooth, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 75: Bell Pad Morph
    // A soft, dreamy pad with sine and triangle waves
    {
        56, 4, WSTA_SEQ_RETRIGGER_ADSR, // Bell Tone Start
        60, 4, 0,                       // Classic Pad Morph
        56, 4, WSTA_SEQ_SUSTAIN,        // Bell Tone Sustain
        60, 4, WSTA_SEQ_END,            // Classic Pad Release
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 76: Rhythmic Noise
    // A rhythmic sequence with noise and mutes for a percussive effect
    {
        39, 1, 0,              // Noise
        39, 1, WSTA_SEQ_MUTE,  // Mute
        39, 1, 0,              // Noise
        39, 1, WSTA_SEQ_MUTE | WSTA_SEQ_LOOP,  // Mute, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 77: Warped Arcade Loop
    // A strange, otherworldly sequence with glitchy sines and noise
    {
        25, 2, WSTA_SEQ_PITCH_SCALE, // Warp Speed High
        29, 2, WSTA_SEQ_RETRIGGER_ADSR, // Laser Malfunction
        25, 2, WSTA_SEQ_PITCH_SCALE, // Warp Speed Low
        29, 2, WSTA_SEQ_LOOP,        // Laser Malfunction Loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 78: Thunderstorm
    // A dramatic sequence with noise and pitch scaling for a storm effect
    {
        39, 4, WSTA_SEQ_PITCH_SCALE,  // Noise, pitch scaled
        39, 4, WSTA_SEQ_PITCH_SCALE,  // Noise, pitch scaled (lower)
        39, 4, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP,  // Noise, pitch scaled (lowest), loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 79: Cosmic Wind
    // A windy, space-like sequence with noise and pitch scaling
    {
        39, 4, WSTA_SEQ_PITCH_SCALE,  // Noise, pitch scaled
        39, 4, WSTA_SEQ_PITCH_SCALE,  // Noise, pitch scaled (higher)
        39, 4, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP,  // Noise, pitch scaled (highest), loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
        // Sequence 80: Alien Landscape
    // Evolving alien soundscape with chatter-like tones and LFO resets
    {
        24, 1, WSTA_SEQ_RESET_LFO,  // Alien chatter, reset LFO for wobble
        26, 1, 0,                   // Overload spark
        24, 2, WSTA_SEQ_RESET_LFO,  // Extended chatter
        26, 1, WSTA_SEQ_MUTE,       // Spark with sudden silence
        28, 3, WSTA_SEQ_SUSTAIN,    // Humming drone, sustained
        24, 1, WSTA_SEQ_RESET_LFO,  // Chatter returns
        26, 2, WSTA_SEQ_PITCH_SCALE,// Spark with pitch shift
        28, 4, WSTA_SEQ_LOOP,       // Drone loops back
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 81: Glitchy Rhythm
    // Complex glitch rhythm with mutes and retriggers
    {
        35, 1, WSTA_SEQ_RETRIGGER_ADSR, // Glitch sine, sharp attack
        35, 1, WSTA_SEQ_MUTE,           // Sudden mute
        36, 2, 0,                       // Glitch square
        35, 1, WSTA_SEQ_MUTE,           // Mute again
        36, 1, WSTA_SEQ_RETRIGGER_ADSR, // Square retrigger
        35, 2, WSTA_SEQ_PITCH_SCALE,    // Sine pitch shift
        36, 1, WSTA_SEQ_MUTE,           // Square mute
        35, 3, WSTA_SEQ_LOOP,           // Sine loops with glitch
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 82: Ethereal Bells
    // Bell-like tones with sustain, pitch scaling, and retriggers
    {
        42, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_SUSTAIN, // Chime, sustained
        42, 1, WSTA_SEQ_PITCH_SCALE,                       // Higher chime
        43, 2, WSTA_SEQ_RETRIGGER_ADSR,                    // Brighter chime
        42, 1, WSTA_SEQ_SUSTAIN,                           // Sustained chime
        43, 1, WSTA_SEQ_PITCH_SCALE,                       // Pitch-shifted chime
        42, 2, WSTA_SEQ_RETRIGGER_ADSR,                    // Retriggered chime
        43, 1, WSTA_SEQ_SUSTAIN,                           // Final sustained chime
        42, 1, WSTA_SEQ_END,                               // End with base chime
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 83: Morphing Pad
    // Morphing pad with waveform transitions and sustain
    {
        0, 4, WSTA_SEQ_SUSTAIN,     // Sine, sustained
        2, 3, 0,                    // Triangle transition
        6, 2, WSTA_SEQ_SUSTAIN,     // Sawtooth, sustained
        8, 2, WSTA_SEQ_PITCH_SCALE, // Pulse, pitch-shifted
        6, 3, WSTA_SEQ_SUSTAIN,     // Sawtooth again
        2, 2, 0,                    // Triangle back
        0, 4, WSTA_SEQ_SUSTAIN,     // Sine sustained
        8, 1, WSTA_SEQ_END,         // Pulse ends
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 84: Percussive Groove
    // Layered percussive groove with retriggers and mutes
    {
        40, 1, WSTA_SEQ_RETRIGGER_ADSR, // Wooden hit
        42, 1, WSTA_SEQ_MUTE,           // Chime muted
        41, 1, WSTA_SEQ_RETRIGGER_ADSR, // Metal hit
        40, 2, 0,                       // Wooden hit extended
        42, 1, WSTA_SEQ_RETRIGGER_ADSR, // Chime retrigger
        41, 1, WSTA_SEQ_MUTE,           // Metal muted
        40, 1, WSTA_SEQ_RETRIGGER_ADSR, // Wooden hit again
        42, 2, WSTA_SEQ_LOOP,           // Chime loops
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 85: Dreamy Arpeggio
    // Arpeggio with pitch scaling and LFO resets
    {
        0, 1, WSTA_SEQ_PITCH_SCALE,     // Sine base
        2, 1, WSTA_SEQ_PITCH_SCALE,     // Triangle up
        6, 1, WSTA_SEQ_PITCH_SCALE,     // Sawtooth higher
        8, 1, WSTA_SEQ_RESET_LFO,       // Pulse with LFO reset
        6, 1, WSTA_SEQ_PITCH_SCALE,     // Sawtooth down
        2, 1, WSTA_SEQ_PITCH_SCALE,     // Triangle down
        0, 1, WSTA_SEQ_RESET_LFO,       // Sine with LFO reset
        8, 1, WSTA_SEQ_LOOP,            // Pulse loops
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 86: Cosmic Drone
    // Deep drone with pitch scaling and sustain
    {
        6, 4, WSTA_SEQ_SUSTAIN,         // Sawtooth base
        8, 3, WSTA_SEQ_PITCH_SCALE,     // Pulse pitch up
        6, 4, WSTA_SEQ_SUSTAIN,         // Sawtooth sustained
        8, 2, WSTA_SEQ_PITCH_SCALE,     // Pulse pitch higher
        10, 3, WSTA_SEQ_SUSTAIN,        // Noise drone
        8, 2, WSTA_SEQ_PITCH_SCALE,     // Pulse down
        6, 4, WSTA_SEQ_SUSTAIN,         // Sawtooth again
        10, 1, WSTA_SEQ_END,            // Noise ends
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 87: Retro Game Sound
    // Expanded retro game sequence with more steps
    {
        4, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Square hit
        2, 1, 0,                        // Triangle
        4, 2, WSTA_SEQ_PITCH_SCALE,     // Square pitch up
        2, 1, WSTA_SEQ_MUTE,            // Triangle muted
        4, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Square again
        2, 2, WSTA_SEQ_PITCH_SCALE,     // Triangle pitch up
        4, 1, 0,                        // Square base
        2, 1, WSTA_SEQ_LOOP,            // Triangle loops
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 88: Synth Brass 2
    // Bold brass with layered waveforms
    {
        6, 2, WSTA_SEQ_SUSTAIN,         // Sawtooth base
        4, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Square punch
        6, 2, WSTA_SEQ_PITCH_SCALE,     // Sawtooth pitch up
        4, 1, WSTA_SEQ_SUSTAIN,         // Square sustained
        8, 2, 0,                        // Pulse layer
        6, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Sawtooth retrigger
        4, 2, WSTA_SEQ_SUSTAIN,         // Square sustained
        8, 1, WSTA_SEQ_END,             // Pulse ends
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 89: Electric Piano
    // Rich electric piano with bell-like decay
    {
        0, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Sine attack
        2, 1, WSTA_SEQ_SUSTAIN,         // Triangle sustain
        42, 1, WSTA_SEQ_RETRIGGER_ADSR, // Chime layer
        0, 1, 0,                        // Sine decay
        2, 1, WSTA_SEQ_SUSTAIN,         // Triangle again
        42, 1, WSTA_SEQ_PITCH_SCALE,    // Chime pitch up
        0, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Sine retrigger
        2, 1, WSTA_SEQ_END,             // Triangle ends
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 90: Funky Bass 2
    // Groovy bass with pitch bends and retriggers
    {
        4, 1, WSTA_SEQ_PITCH_SCALE,     // Square base
        6, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Sawtooth punch
        4, 1, WSTA_SEQ_PITCH_SCALE,     // Square pitch up
        6, 1, 0,                        // Sawtooth
        4, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Square retrigger
        6, 1, WSTA_SEQ_PITCH_SCALE,     // Sawtooth pitch up
        4, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Square again
        6, 1, WSTA_SEQ_LOOP,            // Sawtooth loops
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 91: Sweeping Pad 2
    // Sweeping pad with pitch scaling and sustain
    {
        6, 2, WSTA_SEQ_PITCH_SCALE,     // Sawtooth low
        8, 2, WSTA_SEQ_SUSTAIN,         // Pulse sustained
        6, 2, WSTA_SEQ_PITCH_SCALE,     // Sawtooth mid
        8, 2, WSTA_SEQ_PITCH_SCALE,     // Pulse pitch up
        6, 2, WSTA_SEQ_SUSTAIN,         // Sawtooth sustained
        8, 2, WSTA_SEQ_PITCH_SCALE,     // Pulse higher
        6, 2, WSTA_SEQ_PITCH_SCALE,     // Sawtooth high
        8, 2, WSTA_SEQ_END,             // Pulse ends
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 92: Hybrid EP
    {
        59, 1, WSTA_SEQ_RETRIGGER_ADSR, // EP Start
        62, 2, WSTA_SEQ_PITCH_SCALE,    // Hybrid Wave high accent
        59, 4, WSTA_SEQ_SUSTAIN,        // EP Sustain
        62, 2, WSTA_SEQ_END,            // Hybrid release
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 93: Retro Arp
    // Fast arpeggio with varied waveforms
    {
        4, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Square attack
        2, 1, 0,                        // Triangle
        6, 1, WSTA_SEQ_PITCH_SCALE,     // Sawtooth pitch up
        8, 1, 0,                        // Pulse
        4, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Square again
        2, 1, WSTA_SEQ_PITCH_SCALE,     // Triangle pitch up
        6, 1, 0,                        // Sawtooth
        8, 1, WSTA_SEQ_LOOP,            // Pulse loops
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 94: Vintage Organ
    // Percussive organ with layered tones
    {
        0, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Sine attack
        4, 1, WSTA_SEQ_SUSTAIN,         // Square sustain
        6, 1, WSTA_SEQ_PITCH_SCALE,     // Sawtooth pitch up
        0, 1, 0,                        // Sine
        4, 1, WSTA_SEQ_SUSTAIN,         // Square sustained
        6, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Sawtooth retrigger
        0, 1, WSTA_SEQ_PITCH_SCALE,     // Sine pitch up
        4, 1, WSTA_SEQ_END,             // Square ends
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 95: Retro Lead
    // Vibrato lead with LFO resets and pitch scaling
    {
        6, 1, WSTA_SEQ_RESET_LFO,       // Sawtooth with LFO
        8, 1, WSTA_SEQ_PITCH_SCALE,     // Pulse pitch up
        6, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Sawtooth retrigger
        8, 1, 0,                        // Pulse
        6, 1, WSTA_SEQ_RESET_LFO,       // Sawtooth LFO reset
        8, 1, WSTA_SEQ_PITCH_SCALE,     // Pulse pitch up
        6, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Sawtooth retrigger
        8, 1, WSTA_SEQ_LOOP,            // Pulse loops
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
        // Sequence 96: Gentle Breeze
    // A serene blend of soft sine waves and wind tones with occasional gusts
    {
        0, 4, 0, 0, 4, 0, 0, 4, 0, 0, 4, 0,              // Steps 0-3: Sine wave, calm start
        47, 4, 0, 47, 4, 0, 47, 4, 0, 47, 4, 0,         // Steps 4-7: Wind Through Trees
        26, 1, WSTA_SEQ_RETRIGGER_ADSR, 47, 4, 0,       // Step 8: Spark gust
        47, 4, 0, 47, 4, 0,                              // Steps 9-10: Wind continues
        0, 4, WSTA_SEQ_SUSTAIN,                         // Step 11: Sine sustains
        47, 4, WSTA_SEQ_PITCH_SCALE,                    // Step 12: Wind rises
        26, 2, WSTA_SEQ_RETRIGGER_ADSR,                 // Step 13: Stronger gust
        47, 4, WSTA_SEQ_LOOP,                           // Step 14: Wind loops
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 97: Crackling Campfire
    // Warm drone with irregular percussive crackles
    {
        39, 1, WSTA_SEQ_RETRIGGER_ADSR, 50, 4, 0,       // Step 0: Noise crackle, bass drone
        39, 1, WSTA_SEQ_MUTE, 50, 3, 0,                 // Step 1: Silent crackle, drone
        39, 1, WSTA_SEQ_RETRIGGER_ADSR, 50, 4, 0,       // Step 2: Crackle, drone
        39, 1, WSTA_SEQ_MUTE, 50, 2, 0,                 // Step 3: Silent, shorter drone
        39, 1, WSTA_SEQ_RETRIGGER_ADSR, 50, 4, WSTA_SEQ_SUSTAIN, // Step 4: Crackle, sustain drone
        39, 1, WSTA_SEQ_RETRIGGER_ADSR, 50, 4, WSTA_SEQ_LOOP,    // Step 5: Crackle, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 98: Wobbling Sub Bass
    // Deep, pulsating bass with wobbling pitch
    {
        50, 4, WSTA_SEQ_RESET_LFO, 50, 4, 0,            // Steps 0-1: Bass with LFO wobble
        50, 4, WSTA_SEQ_RESET_LFO, 50, 4, 0,            // Steps 2-3: Wobble repeats
        50, 4, WSTA_SEQ_SUSTAIN,                        // Step 4: Sustain bass
        50, 4, WSTA_SEQ_LOOP,                           // Step 5: Loop wobble
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 99: Fluttering Leaves
    // Light, rapid sounds mimicking leaves in the wind
    {
        46, 1, WSTA_SEQ_RETRIGGER_ADSR, 46, 1, WSTA_SEQ_MUTE,  // Step 0: Droplet flutter, mute
        46, 1, WSTA_SEQ_RETRIGGER_ADSR, 46, 1, WSTA_SEQ_MUTE,  // Step 1: Repeat
        46, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Step 2: Pitch-shifted flutter
        46, 1, WSTA_SEQ_MUTE,                           // Step 3: Mute
        46, 2, WSTA_SEQ_SUSTAIN,                        // Step 4: Sustain flutter
        46, 1, WSTA_SEQ_END,                            // Step 5: Short end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 100: Dancing Lights
    // Rhythmic, twinkling chimes with a playful dance
    {
        42, 1, 0, 42, 1, WSTA_SEQ_MUTE,                // Step 0: Chime, mute
        42, 1, WSTA_SEQ_PITCH_SCALE, 42, 1, WSTA_SEQ_MUTE, // Step 1: Pitch-shifted chime, mute
        42, 2, 0, 42, 1, WSTA_SEQ_MUTE,                // Step 2: Longer chime, mute
        42, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP,   // Step 3: Pitch-shifted chime, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 101: Choir in the Cathedral
    // Sustained vocal tones with harmonic depth
    {
        44, 4, WSTA_SEQ_SUSTAIN,                        // Step 0: Vocal "Ah", sustain
        44, 4, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_SUSTAIN, // Step 1: Detuned vocal, sustain
        44, 4, WSTA_SEQ_RESET_LFO,                      // Step 2: Vocal with vibrato
        44, 4, WSTA_SEQ_LOOP,                           // Step 3: Loop choir
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 102: Whispering Winds
    // Ethereal wind tones with a whispering quality
    {
        47, 4, WSTA_SEQ_PITCH_SCALE, 47, 4, WSTA_SEQ_PITCH_SCALE, // Steps 0-1: Wind, pitch shifted
        47, 4, WSTA_SEQ_SUSTAIN,                        // Step 2: Sustain wind
        47, 4, WSTA_SEQ_LOOP,                           // Step 3: Loop wind
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 103: Cackling Witch
    // Sharp cackles paired with eerie vocal tones
    {
        39, 1, WSTA_SEQ_RETRIGGER_ADSR, 44, 2, 0,       // Step 0: Noise cackle, vocal
        39, 1, WSTA_SEQ_MUTE, 44, 1, WSTA_SEQ_PITCH_SCALE, // Step 1: Silent cackle, pitch-shifted vocal
        39, 1, WSTA_SEQ_RETRIGGER_ADSR, 44, 2, WSTA_SEQ_LOOP, // Step 2: Cackle, vocal loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 104: Wobbling Gelatin
    // Squishy, wobbly bass with dynamic shifts
    {
        50, 2, WSTA_SEQ_RESET_LFO, 50, 2, 0,            // Steps 0-1: Bass wobble
        50, 2, WSTA_SEQ_MUTE, 50, 2, WSTA_SEQ_RESET_LFO, // Step 2: Mute, wobble reset
        50, 4, WSTA_SEQ_SUSTAIN,                        // Step 3: Sustain wobble
        50, 2, WSTA_SEQ_END,                            // Step 4: Short end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 105: Fluttering Butterfly
    // Delicate, rapid flutters with a sustained tone
    {
        45, 1, WSTA_SEQ_RETRIGGER_ADSR, 45, 1, WSTA_SEQ_MUTE, // Step 0: Bird call flutter, mute
        45, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Step 1: Pitch-shifted flutter
        45, 1, WSTA_SEQ_MUTE,                           // Step 2: Mute
        45, 2, WSTA_SEQ_SUSTAIN,                        // Step 3: Sustain flutter
        45, 1, WSTA_SEQ_END,                            // Step 4: Short end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 106: Dancing on Clouds
    // Airy, flowing sequence with smooth transitions
    {
        0, 4, WSTA_SEQ_SUSTAIN, 2, 4, 0,                // Steps 0-1: Sine sustain, triangle
        6, 2, WSTA_SEQ_SUSTAIN, 8, 2, WSTA_SEQ_PITCH_SCALE, // Steps 2-3: Sawtooth, pulse
        6, 3, WSTA_SEQ_SUSTAIN, 2, 2, 0,                // Steps 4-5: Sawtooth sustain, triangle
        0, 4, WSTA_SEQ_SUSTAIN, 8, 1, WSTA_SEQ_END,     // Steps 6-7: Sine sustain, pulse end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 107: Singing Sirens
    // Melodic vocal sequence with a haunting allure
    {
        44, 2, WSTA_SEQ_PITCH_SCALE, 44, 2, 0,          // Steps 0-1: Vocal with pitch shift
        44, 2, WSTA_SEQ_PITCH_SCALE, 44, 2, 0,          // Steps 2-3: Repeat
        44, 4, WSTA_SEQ_SUSTAIN,                        // Step 4: Sustain vocal
        44, 4, WSTA_SEQ_LOOP,                           // Step 5: Loop siren call
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 108: Soft Rain
    // Gentle droplets with a soothing wind backdrop
    {
        46, 1, WSTA_SEQ_RETRIGGER_ADSR, 46, 1, WSTA_SEQ_MUTE, // Step 0: Droplet, mute
        46, 1, WSTA_SEQ_RETRIGGER_ADSR, 46, 1, WSTA_SEQ_MUTE, // Step 1: Repeat
        47, 4, WSTA_SEQ_SUSTAIN,                        // Step 2: Wind sustains
        46, 1, WSTA_SEQ_RETRIGGER_ADSR, 47, 4, WSTA_SEQ_LOOP, // Step 3: Droplet, wind loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 109: Crackling Thunder
    // Powerful bass rumble with sharp thunder cracks
    {
        50, 4, 0, 39, 1, WSTA_SEQ_RETRIGGER_ADSR,       // Step 0: Bass, thunder crack
        50, 4, WSTA_SEQ_SUSTAIN, 39, 1, WSTA_SEQ_RETRIGGER_ADSR, // Step 1: Sustain bass, crack
        50, 4, WSTA_SEQ_END,                            // Step 2: End with rumble
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 110: Stutter Edit Pad
    {
        60, 2, WSTA_SEQ_RETRIGGER_ADSR, // Pad Hit
        60, 1, WSTA_SEQ_MUTE,
        60, 1, WSTA_SEQ_RETRIGGER_ADSR,
        60, 2, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Pitched Hit
        60, 1, WSTA_SEQ_MUTE,
        60, 4, WSTA_SEQ_SUSTAIN, // Sustain Pad
        60, 2, WSTA_SEQ_LOOP,    // Loop short hit on release? Or just END. Let's end. WSTA_SEQ_END
        // Padding
        0,0,WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 111: Fluttering Heartbeat
    // Rhythmic pulses with a fluttering overlay
    {
        45, 1, WSTA_SEQ_RETRIGGER_ADSR, 45, 1, WSTA_SEQ_MUTE, // Step 0: Pulse, mute
        45, 1, WSTA_SEQ_RETRIGGER_ADSR, 45, 1, WSTA_SEQ_MUTE, // Step 1: Repeat
        45, 2, WSTA_SEQ_SUSTAIN,                        // Step 2: Sustain pulse
        45, 1, WSTA_SEQ_LOOP,                           // Step 3: Loop heartbeat
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
        // Sequence 112: Celestial Awakening
    // A serene, evolving pad that builds to a majestic climax
    {
        0, 4, WSTA_SEQ_SUSTAIN,  // Sine, sustained for calm start
        2, 3, 0,                 // Triangle, adding warmth
        6, 2, WSTA_SEQ_SUSTAIN,  // Sawtooth, introducing brightness
        8, 2, WSTA_SEQ_PITCH_SCALE, // Pulse, pitch-shifted for tension
        6, 3, WSTA_SEQ_SUSTAIN,  // Sawtooth, sustained for climax
        2, 2, 0,                 // Triangle, easing back
        0, 4, WSTA_SEQ_SUSTAIN,  // Sine, returning to calm
        8, 1, WSTA_SEQ_END,      // Pulse, gentle end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 113: Nostalgic Echoes
    // Evokes nostalgia with echoing chimes and soft pads
    {
        42, 1, WSTA_SEQ_RETRIGGER_ADSR, 42, 1, WSTA_SEQ_MUTE, // Chime, mute for echo
        42, 1, WSTA_SEQ_RETRIGGER_ADSR, 42, 1, WSTA_SEQ_MUTE, // Repeat chime
        0, 4, WSTA_SEQ_SUSTAIN,                        // Sine pad sustains
        42, 1, WSTA_SEQ_PITCH_SCALE,                   // Pitch-shifted chime
        0, 4, WSTA_SEQ_SUSTAIN,                        // Sine pad continues
        42, 1, WSTA_SEQ_END,                           // Final chime
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 114: Tension and Release
    // Builds tension with rising pitch, resolves to calm
    {
        4, 1, WSTA_SEQ_PITCH_SCALE, 4, 1, WSTA_SEQ_PITCH_SCALE, // Square, rising pitch
        4, 1, WSTA_SEQ_PITCH_SCALE, 4, 1, WSTA_SEQ_PITCH_SCALE, // Continue rising
        39, 1, WSTA_SEQ_RETRIGGER_ADSR,                  // Noise burst for climax
        0, 4, WSTA_SEQ_SUSTAIN,                          // Sine, calm resolution
        0, 4, WSTA_SEQ_END,                              // Sine, gentle end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 115: Meditative Drone
    // Concept: Uses LFO resets and subtle pitch scaling for more movement within the drone. Longer cycles.
    {
        0, 16, WSTA_SEQ_SUSTAIN | WSTA_SEQ_RESET_LFO, // Sine base, long duration, reset LFO for potential vibrato/tremolo
        2, 12, 0,                       // Triangle transition
        0, 16, WSTA_SEQ_SUSTAIN | WSTA_SEQ_RESET_LFO | WSTA_SEQ_PITCH_SCALE, // Sine again, slightly pitched (detuned/phased), LFO reset
        2, 12, WSTA_SEQ_LOOP,           // Triangle loop back
        // Padding
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 116: Rhythmic Pulse
    // Concept: More complex syncopated rhythm using main wood block, higher pitched version, and a muted step.
    // Timing (@1kHz): Loop is 1+1+1+1+1+1 = 6ms. Fast pattern.
    {
        40, 1, WSTA_SEQ_RETRIGGER_ADSR, // Main Wood Block (Wave 40)
        0,  1, WSTA_SEQ_MUTE,           // Silence
        168,1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Higher Wood Block (Wave 40, Size 64 -> 168) Accent
        40, 1, WSTA_SEQ_RETRIGGER_ADSR, // Main Wood Block
        0,  1, WSTA_SEQ_MUTE,           // Silence
        40, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_LOOP, // Main Wood Block & Loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 117: Vocal Formant Shift
    {
        44, 4, WSTA_SEQ_PITCH_SCALE, // Vocal "Ah" High
        44, 4, 0,                    // Vocal "Ah" Normal
        44, 4, WSTA_SEQ_SUSTAIN,     // Sustain Normal
        44, 4, WSTA_SEQ_PITCH_SCALE, // High on Release
        44, 4, WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 118: Metal Scrape Loop
    {
        55, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Metal Impact High
        53, 2, 0, // Cymbal shimmer
        55, 1, WSTA_SEQ_RETRIGGER_ADSR, // Metal Impact Low
        53, 2, WSTA_SEQ_LOOP, // Cymbal Loop
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 119: Oceanic Waves
    // Mimics the ebb and flow of ocean waves
    {
        47, 4, WSTA_SEQ_SUSTAIN, 47, 4, WSTA_SEQ_SUSTAIN, // Wind, sustained for wave crash
        47, 4, WSTA_SEQ_MUTE,                           // Mute for wave receding
        47, 4, WSTA_SEQ_LOOP,                           // Loop wave cycle
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 120: Quantized Arp
    {
        63, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Quant Saw High
        63, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Quant Saw Mid
        63, 1, WSTA_SEQ_RETRIGGER_ADSR,                       // Quant Saw Low
        63, 1, WSTA_SEQ_MUTE,
        63, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_LOOP, // Loop Low
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 121: Gated Sweep Noise
    {
        79, 2, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Noise Rise Gate 1
        79, 1, WSTA_SEQ_MUTE,
        78, 2, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Noise Drop Gate 2
        78, 1, WSTA_SEQ_MUTE,
        79, 2, WSTA_SEQ_SUSTAIN, // Sustain Rise
        78, 2, WSTA_SEQ_END,     // End Drop
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 122: Electric Buzz
    {
        4, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Square buzz attack
        4, 1, WSTA_SEQ_SUSTAIN,         // Sustain buzz
        39, 1, WSTA_SEQ_END,            // Noise decay
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 123: Sci-Fi Blaster
    {
        6, 1, WSTA_SEQ_PITCH_SCALE,     // Sawtooth high pitch
        6, 1, WSTA_SEQ_PITCH_SCALE,     // Lower pitch
        39, 1, WSTA_SEQ_RETRIGGER_ADSR, // Noise burst with envelope
        6, 1, WSTA_SEQ_END,             // Final saw decay
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 124: Pulsing Alarm
    {
        4, 2, WSTA_SEQ_RESET_LFO,       // Square pulse with LFO reset
        4, 2, WSTA_SEQ_MUTE,            // Silence
        4, 2, WSTA_SEQ_RESET_LFO,       // Pulse again
        4, 2, WSTA_SEQ_MUTE | WSTA_SEQ_LOOP, // Silence, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 125: Echoing Ping
    {
        42, 1, WSTA_SEQ_RETRIGGER_ADSR, // Chime attack
        42, 1, WSTA_SEQ_MUTE,           // Short silence
        42, 1, WSTA_SEQ_PITCH_SCALE,    // Echo with pitch shift
        42, 1, WSTA_SEQ_MUTE,           // Silence
        42, 1, WSTA_SEQ_END,            // Final echo
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 126: Teleport Malfunction
    // Concept: Starts clean, glitches heavily, then cuts out abruptly.
    {
        192, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Sine High Pitch (Wave 0, Size 32 -> 192) Start sound
        35,  1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_RESET_LFO,   // Glitch Sine (Wave 35)
        39,  1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Noise Blast (Wave 39) Pitch up
        36,  1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_RESET_LFO,   // Overfolded Saw (Wave 36)
        163, 1, WSTA_SEQ_MUTE,                                  // Glitch Sine (Wave 35, Size 64 -> 163) Muted Glitch
        39,  1, WSTA_SEQ_RETRIGGER_ADSR,                       // Noise Blast repeat
        164, 1, WSTA_SEQ_MUTE | WSTA_SEQ_RESET_LFO,            // Overfolded Saw (Wave 36, Size 64 -> 164) Muted Glitch LFO Reset
        35,  1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_END,        // Glitch Sine final gasp
        // Padding
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 127: Retro Synth Lead
    {
        6, 2, WSTA_SEQ_RESET_LFO,       // Sawtooth with LFO vibrato
        8, 2, WSTA_SEQ_SUSTAIN,         // Pulse sustain
        6, 2, WSTA_SEQ_PITCH_SCALE,     // Saw pitch up
        8, 2, WSTA_SEQ_LOOP,            // Pulse loops
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 128: Funky Groove
    {
        4, 1, WSTA_SEQ_RETRIGGER_ADSR,  // Square punch
        2, 1, WSTA_SEQ_MUTE,            // Silence
        6, 1, WSTA_SEQ_PITCH_SCALE,     // Saw pitch bend
        4, 1, WSTA_SEQ_LOOP,            // Square loops
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 129: Crystal Shimmer
    {
        42, 2, WSTA_SEQ_RETRIGGER_ADSR, // Chime attack
        42, 2, WSTA_SEQ_PITCH_SCALE,    // Pitch-shifted shimmer
        42, 2, WSTA_SEQ_SUSTAIN,        // Sustain shimmer
        42, 1, WSTA_SEQ_END,            // Fade out
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 130: Fragmented Memory Loop
    // Concept: Short, disconnected bursts of different sounds looping.
    {
        56, 1, WSTA_SEQ_RETRIGGER_ADSR, // Bell Tone (Wave 56) fragment 1
        0,  1, WSTA_SEQ_MUTE,           // Silence
        59, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Electric Piano (Wave 59) fragment 2, pitched
        0,  1, WSTA_SEQ_MUTE,           // Silence
        40, 1, WSTA_SEQ_RETRIGGER_ADSR, // Wooden Percussion (Wave 40) fragment 3
        0,  1, WSTA_SEQ_MUTE,           // Silence
        35, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP, // Glitch Sine (Wave 35) fragment 4, pitched & loop
        0,  0, WSTA_SEQ_END,           // End marker (sequence loops before reaching here)
        // Padding
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 131: Percussive Hit
    {
        39, 1, WSTA_SEQ_RETRIGGER_ADSR, // Noise hit
        4, 1, WSTA_SEQ_END,             // Square decay
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 132: Sweeping Laser
    {
        6, 2, WSTA_SEQ_PITCH_SCALE,     // Saw sweep up
        6, 2, WSTA_SEQ_PITCH_SCALE,     // Higher sweep
        6, 2, WSTA_SEQ_PITCH_SCALE,     // Peak pitch
        6, 1, WSTA_SEQ_END,             // Quick decay
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 133: Bouncing Echo
    {
        42, 1, WSTA_SEQ_RETRIGGER_ADSR, // Chime bounce
        42, 1, WSTA_SEQ_MUTE,           // Silence
        42, 1, WSTA_SEQ_PITCH_SCALE,    // Echo bounce
        42, 1, WSTA_SEQ_MUTE,           // Silence
        42, 1, WSTA_SEQ_END,            // Final bounce
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 134: Warped Signal
    {
        35, 1, WSTA_SEQ_RESET_LFO,      // Glitch sine with LFO
        35, 1, WSTA_SEQ_PITCH_SCALE,    // Pitch warp
        39, 1, WSTA_SEQ_RETRIGGER_ADSR, // Noise interference
        35, 1, WSTA_SEQ_LOOP,           // Glitch loops
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 135: Mechanical Click
    {
        39, 1, WSTA_SEQ_RETRIGGER_ADSR, // Noise click
        4, 1, WSTA_SEQ_MUTE,            // Silence
        4, 1, WSTA_SEQ_END,             // Square tick
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 136: Hypnotic Drone
    {
        0, 4, WSTA_SEQ_SUSTAIN,         // Sine drone
        2, 4, WSTA_SEQ_RESET_LFO,       // Triangle with LFO
        0, 4, WSTA_SEQ_SUSTAIN,         // Sine again
        2, 4, WSTA_SEQ_LOOP,            // Triangle loops
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 137: Laser Gun Charge
    {
        0, 2, WSTA_SEQ_PITCH_SCALE, // Sine low pitch start
        0, 2, WSTA_SEQ_PITCH_SCALE, // Sine mid pitch
        0, 2, WSTA_SEQ_PITCH_SCALE, // Sine high pitch charge
        39, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_END, // Noise Blast fire
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 138: Space Drift
    {
        0, 4, WSTA_SEQ_SUSTAIN,         // Sine drift
        6, 2, WSTA_SEQ_PITCH_SCALE,     // Saw sweep
        0, 4, WSTA_SEQ_SUSTAIN,         // Sine again
        6, 2, WSTA_SEQ_LOOP,            // Saw loops
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 139: Glitch Pulse
    {
        35, 1, WSTA_SEQ_RETRIGGER_ADSR, // Glitch pulse
        35, 1, WSTA_SEQ_MUTE,           // Silence
        35, 1, WSTA_SEQ_PITCH_SCALE,    // Pitch glitch
        35, 1, WSTA_SEQ_LOOP,           // Loop glitch
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 140: Echo Drop
    {
        42, 1, WSTA_SEQ_RETRIGGER_ADSR, // Chime drop
        42, 1, WSTA_SEQ_MUTE,           // Silence
        42, 1, WSTA_SEQ_PITCH_SCALE,    // Echo drop
        42, 1, WSTA_SEQ_END,            // Final echo
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 141: Synth Bass Thump
    {
        50, 2, WSTA_SEQ_RETRIGGER_ADSR, // Bass thump
        50, 2, WSTA_SEQ_SUSTAIN,        // Sustain bass
        50, 1, WSTA_SEQ_END,            // Quick decay
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 142: Fluttering Spark
    {
        42, 1, WSTA_SEQ_RETRIGGER_ADSR, // Chime flutter
        42, 1, WSTA_SEQ_MUTE,           // Silence
        42, 1, WSTA_SEQ_PITCH_SCALE,    // Pitch flutter
        42, 1, WSTA_SEQ_END,            // Final spark
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 143: Rising Tension
    {
        6, 2, WSTA_SEQ_PITCH_SCALE,     // Saw rise
        6, 2, WSTA_SEQ_PITCH_SCALE,     // Higher rise
        6, 2, WSTA_SEQ_PITCH_SCALE,     // Peak tension
        39, 1, WSTA_SEQ_END,            // Noise release
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 144: Engine Ignition Failure
    // Concept: Sound tries to start, sputters noisily, then fails.
    {
        0,   4, WSTA_SEQ_RETRIGGER_ADSR, // Low Sine rumble (Wave 0)
        32,  1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Razor Pulse (Wave 32) attempt 1, rising pitch
        39,  1, WSTA_SEQ_MUTE | WSTA_SEQ_RETRIGGER_ADSR,       // Noise Sputter (Wave 39) Muted but retriggers envelope? Maybe not useful. Use Mute only.
                                                                // Corrected: 39, 1, WSTA_SEQ_MUTE
        32,  1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Razor Pulse attempt 2, lower pitch
        39,  1, WSTA_SEQ_MUTE,                                  // Noise Sputter 2 (Mute)
        26,  2, WSTA_SEQ_RETRIGGER_ADSR,                       // Overload Spark (Wave 26) Final attempt/short
        0,   4, WSTA_SEQ_MUTE | WSTA_SEQ_END,                  // Silence / Engine Off
        // Padding
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 145: Wind Gust
    {
        47, 2, WSTA_SEQ_RETRIGGER_ADSR, // Wind rise
        47, 2, WSTA_SEQ_SUSTAIN,        // Wind peak
        47, 1, WSTA_SEQ_END,            // Wind fade
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 146: Bubble Pop
    {
        42, 1, WSTA_SEQ_RETRIGGER_ADSR, // Chime bubble
        42, 1, WSTA_SEQ_PITCH_SCALE,    // Pitch pop
        39, 1, WSTA_SEQ_END,            // Noise burst
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 147: Static Burst
    // Concept: Uses different noise/harsh waves and sizes for a more textured static/crackle, with LFO interaction.
    // Timing (@1kHz): Loop is 1+1+1+1+1+1 = 6ms. Fast texture.
    {
        39, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_RESET_LFO,   // Noise Blast (Wave 39) - LFO Sync start
        103,1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Harmonic Noise (Wave 39, Size 128 -> 103) High pitched crackle
        37, 1, WSTA_SEQ_RETRIGGER_ADSR,                       // Clipped Chaos (Wave 37) Harsh static element
        167,1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Noise Blast (Wave 39, Size 64 -> 167) Mid pitched crackle
        0,  1, WSTA_SEQ_MUTE,                                  // Brief silence
        231,1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP, // Harmonic Noise (Wave 39, Size 32 -> 231) Looping crackle
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 148: Pulsating Glow
    {
        0, 2, WSTA_SEQ_RESET_LFO,       // Sine glow with LFO
        0, 2, WSTA_SEQ_SUSTAIN,         // Sustain glow
        0, 1, WSTA_SEQ_END,             // Fade out
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 149: Bouncing Ball
    // Simulates a bouncing ball with decreasing amplitude and pitch
    {
        42, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // High pitch chime
        42, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Slightly lower
        42, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Lower still
        42, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_END, // Lowest, end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 150: Simple Arpeggio
    // A simple arpeggio pattern with ascending and descending notes
    {
        0, 1, WSTA_SEQ_PITCH_SCALE, // Sine low
        2, 1, WSTA_SEQ_PITCH_SCALE, // Triangle mid
        4, 1, WSTA_SEQ_PITCH_SCALE, // Square high
        2, 1, WSTA_SEQ_PITCH_SCALE, // Triangle mid
        0, 1, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP, // Sine low, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 151: Siren
    // A siren effect with alternating high and low pitches
    {
        4, 2, WSTA_SEQ_PITCH_SCALE, // High square
        4, 2, WSTA_SEQ_PITCH_SCALE, // Low square
        4, 2, WSTA_SEQ_PITCH_SCALE, // High square
        4, 2, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_LOOP, // Low square, loop
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 152: Evolving Rhythmic Noise Texture
    // Concept: Noise texture that changes rhythmically using different noise/harsh waves and sizes.
    {
        39, 1, WSTA_SEQ_RETRIGGER_ADSR,                      // Noise Blast (Wave 39, Size 256)
        103,1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Harmonic Noise (Wave 39, Size 128 -> 103) high pitch variant
        37, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_RESET_LFO,   // Clipped Chaos (Wave 37, Size 256) LFO reset for potential modulation
        164,1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Overfolded Saw (Wave 36, Size 64 -> 164) low pitch variant
        39, 1, WSTA_SEQ_MUTE,                                  // Muted Noise Blast
        231,1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Harmonic Noise (Wave 39, Size 32 -> 231) mid pitch variant
        165,1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_RESET_LFO | WSTA_SEQ_LOOP, // Clipped Chaos (Wave 37, Size 64 -> 165) LFO reset & loop
        0, 0, WSTA_SEQ_END,                                    // End marker (sequence loops before reaching here)
        // Padding
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 153: Thunderous Kick
    // Description: A deep, resonant kick drum with a layered noise attack and pitch drop
    {
        39, 1, WSTA_SEQ_RETRIGGER_ADSR,          // Sharp noise attack (waveform 39, noise)
        192,2, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // High pitch sine (size 3, index 0)
        128,3, WSTA_SEQ_PITCH_SCALE,             // Mid pitch sine (size 2, index 0)
        0,  10,WSTA_SEQ_END,                     // Low pitch sustain and decay (size 0, index 0)
        // Padded steps
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 154: Explosive Crash
    // Description: A cymbal-like crash with layered noise and chime effects
    {
        39, 1, WSTA_SEQ_RETRIGGER_ADSR,          // Initial noise burst (waveform 39)
        194,2, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // High chime (size 3, index 2, triangle)
        39, 5, 0,                                // Sustained noise texture
        2,  10,WSTA_SEQ_END,                     // Tonal decay (size 0, index 2, triangle)
        // Padded steps
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 155: Rumble Roll
    // Description: A rolling percussion with alternating hits and pitch variation
    {
        0,  1, WSTA_SEQ_RETRIGGER_ADSR,          // First hit, low sine (size 0, index 0)
        39, 1, WSTA_SEQ_MUTE,                    // Brief silence
        192,1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // High hit (size 3, index 0)
        39, 1, WSTA_SEQ_MUTE,                    // Brief silence
        64, 2, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Mid hit (size 1, index 0)
        0,  5, WSTA_SEQ_END,                     // Final low sustain
        // Padded steps
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 156: Pac-Man Waka (Revised - Longer Duration) ---
    // Concept: Rhythmic muted pulse using a short, slightly bright wave, longer duration per step.
    // Total loop duration: 5+5+5+5 = 20 cycles (~182ms @ 110Hz)
    {
        235, 5, WSTA_SEQ_RETRIGGER_ADSR, // Square wave (Wave 4, Size 64 -> 132), ON duration
        0,   5, WSTA_SEQ_MUTE,           // Mute for OFF duration
        235, 5, WSTA_SEQ_RETRIGGER_ADSR, // Square wave ON
        0,   5, WSTA_SEQ_MUTE | WSTA_SEQ_LOOP, // Mute OFF & Loop
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END
    },
    // Sequence 157: Long Vinyl Scratch
    // Concept: Simulates a vinyl scratch using noise, harsh waves, pitch scaling, and LFO resets.
    // Duration: ~150ms+ @ 100Hz before envelope release.
    {
        103, 2, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Noise Blast (Wave 39, Size 128->103) High pitch fast start
        37,  3, WSTA_SEQ_RESET_LFO,                            // Clipped Chaos (Wave 37) Texture change + LFO
        39,  4, WSTA_SEQ_PITCH_SCALE,                          // Noise Blast (Wave 39, Size 256) Mid pitch slowing down
        165, 3, WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_RESET_LFO,     // Clipped Chaos (Wave 37, Size 64->165) Lower pitch + LFO
        39,  6, 0,                                             // Noise Blast (Wave 39, Size 256) Lowest pitch end drag
        0,   1, WSTA_SEQ_MUTE | WSTA_SEQ_END,                  // Abrupt Mute end
        // Padding
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END,
        0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END, 0,0,WSTA_SEQ_END
    },

    // Sequence 158: Electronic Bassdrum
    // Concept: Sharp tick, rapid sine pitch drop using different wave sizes, longer decay.
    // Duration: ~50ms+ @ 60Hz before envelope release.
    {
        243, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Hi-Hat (Wave 51, Size 32->243) Highest pitch click attack
        192, 1, WSTA_SEQ_PITCH_SCALE,    // Sine (Wave 0, Size 32->192) Highest pitch body start
        128, 1, WSTA_SEQ_PITCH_SCALE,    // Sine (Wave 0, Size 64->128) Mid-high pitch body
        64,  2, WSTA_SEQ_PITCH_SCALE,    // Sine (Wave 0, Size 128->64) Mid-low pitch body
        0,  25, WSTA_SEQ_END,            // Sine (Wave 0, Size 256->0) Low pitch decay/body
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },

    // Sequence 159: Electronic Snare
    // Concept: Sharp noise snap with two quick, slightly detuned tonal elements.
    // Duration: ~15ms @ 200Hz before envelope release.
    {
        39,  1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Noise Blast (Wave 39) sharp, high pitch attack
        192, 1, WSTA_SEQ_PITCH_SCALE,    // Sine (Wave 0, Size 32->192) short high tonal body 1
        130, 1, WSTA_SEQ_PITCH_SCALE,    // Triangle (Wave 2, Size 64->130) short mid tonal body 2 (slightly different timbre/pitch)
        39,  8, 0,                       // Noise Blast (Wave 39, Size 256) decay/rattle
        0,   1, WSTA_SEQ_MUTE | WSTA_SEQ_END, // Mute end
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },

    // Sequence 160: Dance Hand Clap
    // Concept: Rapid succession of noise bursts with slight variation, followed by decay.
    // Duration: ~25ms @ 150Hz before envelope release.
    {
        39,  1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Noise Blast (Wave 39) high pitch burst 1
        167, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Noise Blast (Wave 39, Size 64->167) mid pitch burst 2
        39,  1, WSTA_SEQ_RETRIGGER_ADSR,                       // Noise Blast (Wave 39, Size 256) low pitch burst 3
        103, 1, WSTA_SEQ_RETRIGGER_ADSR,                       // Noise Blast (Wave 39, Size 128->103) mid pitch burst 4
        39, 15, WSTA_SEQ_END,                                  // Noise Blast (Wave 39, Size 256) tail/decay
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },

    // Sequence 161: Dance Tom
    // Concept: Clear attack, strong pitch drop using sine/triangle waves.
    // Duration: ~40ms+ @ 100Hz before envelope release.
    {
        130, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Triangle (Wave 2, Size 64->130) High pitch attack
        64,  2, WSTA_SEQ_PITCH_SCALE,    // Sine (Wave 0, Size 128->64) Mid-high pitch
        0,   3, WSTA_SEQ_PITCH_SCALE,    // Sine (Wave 0, Size 256->0) Mid-low pitch
        0,  20, WSTA_SEQ_END,            // Sine (Wave 0, Size 256->0) Low pitch body/decay
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },

    // Sequence 162: Industrial Zap ---
    // Concept: Harsh, quick sequence with multiple textures and pitch shifts.
    // Duration: ~10ms @ 150Hz before envelope release.
    {
        32,  1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Razor Pulse (Wave 32) high pitch start
        164, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE | WSTA_SEQ_RESET_LFO, // Overfolded Saw (Wave 36, Size 64->164) low pitch LFO glitch
        37,  1, WSTA_SEQ_RETRIGGER_ADSR,                       // Clipped Chaos (Wave 37) mid texture
        231, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Harmonic Noise (Wave 39, Size 32->231) high pitched sizzle
        26,  2, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_END,        // Overload Spark (Wave 26) final burst
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
    // Sequence 163: Most Pleasing Closed Hi-Hat (Revised - Longer Duration) ---
    // Concept: Sharp metallic tick, layered noise decay with longer duration.
    // Total duration: 1 + 9 + 20 = 30 cycles (~68ms @ 440Hz, ~150ms @ 200Hz)
    {
        243, 1, WSTA_SEQ_RETRIGGER_ADSR | WSTA_SEQ_PITCH_SCALE, // Hi-Hat wave (Wave 51, Size 32->243), Highest Pitch

        167, 9, WSTA_SEQ_RETRIGGER_ADSR,                       // Noise Blast (Wave 39, Size 64->167), Retrig to maintain energy
        39, 20, WSTA_SEQ_END,                                  // Noise Blast (Wave 39, Size 256->39), Long decay, End sequence
        0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END,
        0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END, 0, 0, WSTA_SEQ_END
    },
};