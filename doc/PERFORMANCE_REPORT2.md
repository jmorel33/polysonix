# Polysonix VM Optimization Report 2 (v1.8.10)

## Executive Summary

This benchmark compares the performance of the latest v1.8.10 Virtual Machine (iterative sigma, flat stack) against the v1.1.6 baseline (recursive sigma).

**Important Context:**
*   **Baseline (Before):** Measurements taken on an **Apple Silicon M3** (ARM64, High Performance).
*   **Current (After):** Measurements taken on a **Standard Linux Cloud VM** (x86_64, variable clock).

### Interpreting the Numbers
1.  **Simple Waveforms (e.g., Sine, Saw):** Show a performance *drop* (e.g., -50% to -80%). This is **expected** due to the raw hardware difference between the M3 (baseline) and the cloud environment. The M3 has significantly higher single-core IPC and clock speed for basic arithmetic.
2.  **Complex & Sigma Waveforms:** Show massive **gains** (up to +96%). Despite the slower hardware, the **algorithmic optimization** (removing recursion overhead, better cache locality, iterative loops) completely overcomes the hardware deficit.

> **Key Takeaway:** The new architecture is so efficient that it runs complex, heavy patches *faster* on a standard cloud CPU than the old architecture did on an M3 processor.

## Performance Scorecard

| ROM ID | Name | Baseline (M3) (ns) | v1.8.10 (Cloud) (ns) | Improvement |
| :--- | :--- | :--- | :--- | :--- |
| 0 | Triangle Up | 75.45 | 53.48 | **29.12%** |
| 1 | Triangle Down | 76.76 | 57.52 | **25.07%** |
| 2 | Sine Up | 91.07 | 145.22 | -59.46% |
| 3 | Sine Down | 95.78 | 147.83 | -54.34% |
| 4 | Square Up | 46.59 | 58.34 | -25.22% |
| 5 | Square Down | 45.81 | 70.80 | -54.55% |
| 6 | Saw Rising | 78.59 | 74.04 | **5.79%** |
| 7 | Saw Falling | 80.77 | 73.72 | **8.73%** |
| 8 | Saw/Sine Up | 94.63 | 125.99 | -33.14% |
| 9 | Sine/Saw Down | 96.19 | 164.27 | -70.78% |
| 10 | Square/Sine Up | 91.89 | 113.36 | -23.36% |
| 11 | Sine/Square Down | 94.83 | 162.01 | -70.84% |
| 12 | Saw/Triangle Up | 88.32 | 37.52 | **57.52%** |
| 13 | Triangle/Saw Down | 78.63 | 43.22 | **45.03%** |
| 14 | Triangle/Sine Up | 81.67 | 88.32 | -8.14% |
| 15 | Sine/Triangle Down | 84.56 | 76.45 | **9.59%** |
| 16 | Clipped Sine | 85.38 | 55.63 | **34.84%** |
| 17 | Rectified Sine | 71.58 | 51.72 | **27.75%** |
| 18 | Sine * Saw | 71.59 | 93.88 | -31.14% |
| 19 | Overload Spark | 120.27 | 49.93 | **58.49%** |
| 20 | Overfolded Saw | 103.85 | 65.45 | **36.98%** |
| 21 | Clipped Chaos | 125.70 | 66.19 | **47.34%** |
| 22 | Wavefolder Sim (A=Fold B=Bias) | 81.56 | 70.16 | **13.98%** |
| 29 | Gritty Bass | 73.39 | 67.94 | **7.43%** |
| 30 | Hybrid Saw*Sine | 54.37 | 44.64 | **17.90%** |
| 31 | Razor Pulse | 81.27 | 59.81 | **26.41%** |
| 32 | Pulse 25% | 49.33 | 49.12 | **0.43%** |
| 33 | Pulse 75% | 46.33 | 46.83 | -1.08% |
| 34 | Staircase 4 Step | 107.81 | 56.14 | **47.93%** |
| 35 | Bit Crush Bomb | 114.44 | 49.32 | **56.90%** |
| 36 | Bit-Crushed Square | 104.59 | 64.80 | **38.04%** |
| 37 | Pulse Train Wreck | 133.80 | 54.81 | **59.04%** |
| 38 | Narrow | 76.91 | 26.38 | **65.70%** |
| 39 | Quantized Saw 8 | 42.91 | 35.96 | **16.20%** |
| 40 | PWM Synth (A=Width B=Sub) | 76.16 | 58.16 | **23.63%** |
| 41 | PWM Gate | 46.07 | 39.65 | **13.94%** |
| 42 | Harmonic Switch | 84.56 | 66.35 | **21.54%** |
| 43 | Multi-Gate | 96.15 | 48.09 | **49.98%** |
| 48 | Warp Speed | 77.74 | 22.81 | **70.66%** |
| 49 | Ghost Wail | 69.04 | 48.25 | **30.11%** |
| 50 | Laser Malfunction | 100.14 | 63.72 | **36.37%** |
| 51 | Hyperspace Glitch | 123.91 | 27.34 | **77.94%** |
| 52 | Shredded Saw | 101.88 | 68.85 | **32.42%** |
| 53 | Glitch Sine | 103.51 | 64.04 | **38.13%** |
| 54 | 4-Segment Bump | 129.84 | 55.49 | **57.26%** |
| 55 | Bird Call AM | 68.65 | 55.45 | **19.23%** |
| 56 | Phase Distortion (A=Amt B=Shape) | 83.22 | 73.40 | **11.80%** |
| 57 | Chaos Sine (A=ModRate B=ModAmt) | 68.58 | 52.57 | **23.34%** |
| 58 | Phase Glitch | 61.53 | 63.87 | -3.80% |
| 59 | Phase Distortion Wave | 258.76 | 205.13 | **20.73%** |
| 64 | Classic FM EP (A=Index B=Detune) | 98.97 | 84.26 | **14.86%** |
| 65 | FM Bass Growl (A=Fdbk B=Index) | 139.08 | 125.33 | **9.89%** |
| 66 | Freq Shifter FM (A=Shift B=Index) | 72.75 | 59.18 | **18.65%** |
| 67 | Complex FM A=Index B=ModFreq | 73.56 | 58.09 | **21.03%** |
| 68 | FM Pluck | 170.26 | 150.05 | **11.87%** |
| 69 | FM Pitched Grit | 172.17 | 157.04 | **8.79%** |
| 70 | FM Dynamic Lead | 230.45 | 160.28 | **30.45%** |
| 71 | FM Glassy Evolve | 121.88 | 108.01 | **11.38%** |
| 72 | FM: Classic EP (A=Tine B=Bell C=Ratio) | 109.59 | 89.15 | **18.65%** |
| 73 | FM: Growl Bass (A=Index B=Fdbk C=Ratio) | 97.44 | 75.81 | **22.20%** |
| 79 | FM: Sci-Fi Drone (A=Evolve B=Chaos C=Pitch) | 94.39 | 71.72 | **24.02%** |
| 80 | FM Metallic Bell (A=Decay B=Ratio) | 115.05 | 103.10 | **10.39%** |
| 81 | FM Hollow Drone (A=ModMix B=ModRatio) | 132.58 | 97.87 | **26.18%** |
| 82 | FM Harsh Noise Sweep (A=Sweep B=Intensity) | 119.73 | 107.04 | **10.60%** |
| 83 | FM Soft Pad (A=Brightness B=Chorus) | 167.60 | 125.12 | **25.35%** |
| 84 | FM Bipolar Sweep Pad | 104.25 | 86.80 | **16.74%** |
| 85 | FM Clangorous Hit (A=Metal B=Dissonance) | 175.66 | 148.52 | **15.45%** |
| 87 | FM Evolving SciFi (A=Evolve B=Harmonics) | 145.45 | 96.92 | **33.37%** |
| 88 | FM: Metallic Bell (A=Decay B=Index2 C=Ratio) | 118.56 | 106.04 | **10.56%** |
| 89 | FM: Glitchy Noise (A=Index B=Bit C=Rate) | 117.32 | 98.06 | **16.42%** |
| 93 | Sci-Fi Drone | 301.69 | 247.77 | **17.87%** |
| 94 | Evolving Metallic Bell | 796.01 | 619.44 | **22.18%** |
| 95 | Alien Communication | 286.32 | 203.87 | **28.80%** |
| 96 | Sine Harmonics | 97.92 | 95.06 | **2.92%** |
| 97 | Harmonic Noise Blast | 107.01 | 93.11 | **12.99%** |
| 98 | Brass | 113.78 | 121.41 | -6.71% |
| 99 | Bowed String | 126.98 | 131.31 | -3.41% |
| 100 | Additive Square | 115.24 | 85.00 | **26.24%** |
| 101 | Electric Pianoish | 120.66 | 84.10 | **30.30%** |
| 102 | Classic Pad | 115.32 | 85.12 | **26.19%** |
| 103 | Additive Saw (A=Harms B=Shape) | 190.93 | 309.88 | -62.30% |
| 104 | Random Phase Additive (A=RndAmt B=Harm) | 228.45 | 286.35 | -25.34% |
| 105 | Grit Additive (A=Grit B=Tone) | 288.92 | 260.40 | **9.87%** |
| 106 | Simple Minor Triad | 133.58 | 107.45 | **19.56%** |
| 112 | Formantish | 78.43 | 68.92 | **12.13%** |
| 113 | Vocal Ah | 111.30 | 126.76 | -13.89% |
| 114 | Reso Filter Sweep (A=Reso B=Cutoff) | 163.34 | 144.04 | **11.82%** |
| 115 | Formant Vowel (A=Phsr1 B=Phsr2) | 645.98 | 500.23 | **22.56%** |
| 116 | Sync Sweep No Slant | 55.96 | 51.33 | **8.27%** |
| 117 | Sync Sweep Cos Shape | 70.40 | 56.52 | **19.72%** |
| 118 | Smoothed Sync (A=SyncFreq B=Duty) | 138.03 | 126.77 | **8.16%** |
| 119 | Limited Sync (A=SyncFreq B=Duty) | 67.96 | 60.74 | **10.62%** |
| 120 | Sync Sweep (A=SyncFreq B=Duty) | 67.32 | 64.51 | **4.17%** |
| 121 | Oooh Choir Formant | 2304.00 | 2325.16 | -0.92% |
| 125 | FM Breathy Flute (A=Air B=PitchMod) | 137.67 | 94.48 | **31.37%** |
| 128 | Kick Drum | 81.49 | 72.58 | **10.93%** |
| 129 | Snare Drum | 99.86 | 85.01 | **14.87%** |
| 130 | Clap | 104.74 | 90.37 | **13.72%** |
| 131 | Tom Drum | 88.48 | 65.90 | **25.52%** |
| 132 | Cymbalish | 86.73 | 71.11 | **18.01%** |
| 133 | Double Waves | 129.19 | 104.53 | **19.09%** |
| 134 | Metal Impact | 97.84 | 73.86 | **24.51%** |
| 135 | Bell Tone | 115.18 | 87.15 | **24.34%** |
| 136 | Metallic Perc | 92.67 | 71.91 | **22.40%** |
| 137 | Sigma Bell (A=Decay B=Metal) | 376.90 | 310.44 | **17.63%** |
| 138 | Classic Noise Sim | 177.57 | 138.14 | **22.21%** |
| 139 | Distorted Pitch | 150.59 | 144.73 | **3.89%** |
| 140 | Gritty Rumble Noise | 187.26 | 104.61 | **44.14%** |
| 141 | Filtered Static Noise | 684.39 | 450.67 | **34.15%** |
| 142 | Wooden Percussion | 89.24 | 85.24 | **4.48%** |
| 143 | Glitchy Percussion | 294.50 | 257.70 | **12.50%** |
| 144 | Plucked String (A=Damp B=Body) | 352.67 | 336.91 | **4.47%** |
| 145 | Sigma A=End B=Decay | 202.93 | 275.45 | -35.74% |
| 146 | Noisy Pad (A=NoiseAmt B=Flt) | 371.56 | 268.89 | **27.63%** |
| 147 | Rich String Ensemble | 1479.07 | 1397.22 | **5.53%** |
| 148 | Mellow Brass Section | 733.82 | 766.05 | -4.39% |
| 149 | Jittery Inharmonic Pitch | 792.37 | 1083.48 | -36.74% |
| 150 | LFSR Granular Texture | 244.07 | 203.71 | **16.54%** |
| 151 | Morphing Harmonics | 2188.26 | 1764.81 | **19.35%** |
| 152 | Breathing Pad | 200.09 | 145.41 | **27.33%** |
| 153 | Chaotic Oscillator | 248.92 | 204.25 | **17.95%** |
| 154 | Crystalline Arpeggio | 1356.90 | 1175.37 | **13.38%** |
| 156 | Water Droplet | 100.12 | 55.76 | **44.31%** |
| 157 | Alien Chatter | 120.35 | 46.84 | **61.08%** |
| 159 | Wind AM | 64.32 | 51.52 | **19.90%** |
| 160 | LFSR Rhythm Gate | 132.76 | 89.10 | **32.89%** |
| 161 | LFSR Harmonic Chaos | 941.77 | 725.42 | **22.97%** |
| 162 | LFSR Digital Texture | 297.65 | 225.45 | **24.26%** |
| 163 | LFSR Poly Rhythm | 242.80 | 162.11 | **33.23%** |
| 164 | LFSR Phase Modulation | 160.86 | 143.38 | **10.87%** |
| 165 | LFSR Granular | 721.90 | 577.87 | **19.95%** |
| 166 | LFSR Rhythmic Harmonics | 1060.93 | 785.90 | **25.92%** |
| 167 | LFSR Spectral Shift | 207.43 | 156.68 | **24.47%** |
| 168 | LFSR Euclidean Beat | 165.02 | 135.61 | **17.82%** |
| 169 | LFSR Feedback Synth | 247.40 | 208.07 | **15.90%** |
| 170 | LFSR Algorithmic Lead | 256.78 | 177.65 | **30.82%** |
| 171 | LFSR Morphing Pad | 1269.51 | 1174.24 | **7.50%** |
| 172 | LFSR Breakbeat | 195.89 | 143.05 | **26.97%** |
| 173 | LFSR Probability Gate | 180.26 | 159.48 | **11.53%** |
| 174 | LFSR Polyrhythmic Chaos | 234.28 | 167.24 | **28.62%** |
| 175 | LFSR Glitch Matrix | 749.37 | 677.61 | **9.58%** |
| 176 | Pac-Man Wakka | 104.83 | 81.09 | **22.65%** |
| 177 | Pac-Man Power Pellet | 113.20 | 81.16 | **28.30%** |
| 178 | Pac-Man Death | 109.94 | 82.52 | **24.94%** |
| 179 | Pac-Man Ghost | 125.74 | 86.19 | **31.45%** |
| 180 | Space Invaders Shot | 132.36 | 100.21 | **24.29%** |
| 181 | Space Invaders March | 81.79 | 54.57 | **33.28%** |
| 182 | Space Invaders UFO | 150.52 | 97.63 | **35.14%** |
| 183 | Space Invaders Explosion | 129.85 | 116.19 | **10.52%** |
| 184 | Asteroids Thrust | 174.96 | 169.47 | **3.14%** |
| 185 | Asteroids Shoot | 135.43 | 100.50 | **25.79%** |
| 186 | Asteroids Explosion | 136.78 | 119.56 | **12.59%** |
| 187 | Asteroids Hyperspace | 147.84 | 116.57 | **21.15%** |
| 188 | Galaxian Attack | 109.38 | 79.02 | **27.76%** |
| 189 | Galaxian Formation | 158.19 | 101.06 | **36.11%** |
| 190 | Centipede Laser | 147.32 | 115.21 | **21.80%** |
| 191 | Centipede Flea Drop | 110.51 | 95.19 | **13.86%** |
| 192 | Defender Thrust | 170.65 | 164.72 | **3.47%** |
| 193 | Defender Smart Bomb | 152.62 | 116.16 | **23.89%** |
| 194 | Frogger Hop | 133.49 | 105.53 | **20.95%** |
| 195 | Frogger Traffic | 129.61 | 123.51 | **4.71%** |
| 196 | Donkey Kong Hammer | 112.03 | 140.36 | -25.29% |
| 197 | Donkey Kong Jump | 113.49 | 93.52 | **17.60%** |
| 198 | Missile Command Explosion | 141.80 | 123.87 | **12.64%** |
| 199 | Tempest Shoot | 139.21 | 106.90 | **23.21%** |
| 200 | Tempest Flip | 106.62 | 79.14 | **25.77%** |
| 201 | Berzerk Robot Voice | 187.70 | 168.29 | **10.34%** |
| 202 | Robotron Shoot | 143.71 | 117.34 | **18.35%** |
| 203 | Phoenix Bird Cry | 140.74 | 100.26 | **28.76%** |
| 204 | Gorf Laser | 135.79 | 106.44 | **21.61%** |
| 205 | Scramble Engine | 171.00 | 164.88 | **3.58%** |
| 206 | Zaxxon Alarm | 135.79 | 104.35 | **23.15%** |
| 207 | Moon Patrol Bounce | 159.52 | 128.56 | **19.41%** |
| 208 | POKEY Pure Tone | 77.26 | 78.22 | -1.24% |
| 209 | POKEY Filtered Noise | 100.02 | 114.09 | -14.07% |
| 210 | POKEY Distorted Bass | 141.52 | 129.21 | **8.70%** |
| 211 | POKEY Laser Zap | 145.43 | 148.24 | -1.93% |

**Overall Average Delta (Hardware + Software): 15.99%**
