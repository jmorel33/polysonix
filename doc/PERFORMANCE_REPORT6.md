# Polysonix VM v1.9.26 Performance Report

This report compares the execution time of the new Iterative Sigma / Stack-Peek VM (v1.9.26) against the v1.9.25 baseline.
Measurements are averaged over 10 runs to eliminate OS and cache noise.

| Patch ID | Name | v1.9.25 (Before) (ns) | v1.9.26 (After) (ns) | Improvement |
| :--- | :--- | :--- | :--- | :--- |
| 0 | Triangle Up | 49.74 | 49.67 | **0.14%** |
| 1 | Triangle Down | 51.25 | 51.81 | -1.10% |
| 2 | Sine Up | 95.16 | 95.77 | -0.64% |
| 3 | Sine Down | 97.31 | 99.13 | -1.88% |
| 4 | Square Up | 55.58 | 54.53 | **1.88%** |
| 5 | Square Down | 54.49 | 54.51 | -0.04% |
| 6 | Saw Rising | 71.56 | 73.13 | -2.19% |
| 7 | Saw Falling | 71.97 | 73.48 | -2.10% |
| 8 | Saw/Sine Up | 87.27 | 87.26 | **0.01%** |
| 9 | Sine/Saw Down | 92.33 | 92.83 | -0.54% |
| 10 | Square/Sine Up | 79.21 | 78.74 | **0.59%** |
| 11 | Sine/Square Down | 79.48 | 78.94 | **0.69%** |
| 12 | Saw/Triangle Up | 44.10 | 39.89 | **9.55%** |
| 13 | Triangle/Saw Down | 37.70 | 38.19 | -1.29% |
| 14 | Triangle/Sine Up | 68.42 | 67.40 | **1.49%** |
| 15 | Sine/Triangle Down | 71.42 | 67.37 | **5.67%** |
| 16 | Clipped Sine | 48.79 | 49.07 | -0.57% |
| 17 | Rectified Sine | 48.59 | 48.77 | -0.38% |
| 18 | Sine * Saw | 53.82 | 54.87 | -1.95% |
| 19 | Overload Spark | 74.64 | 71.13 | **4.70%** |
| 20 | Overfolded Saw | 63.22 | 63.45 | -0.36% |
| 21 | Clipped Chaos | 73.35 | 72.32 | **1.40%** |
| 22 | Wavefolder Sim (A=Fold B=Bias) | 77.60 | 76.89 | **0.91%** |
| 23 | Triangle Fold | 41.21 | 41.72 | -1.24% |
| 24 | Math: Tanh Drive | 33.71 | 33.84 | -0.40% |
| 25 | Math: Cubic | 32.25 | 32.30 | -0.14% |
| 26 | Math: Rectified | 24.66 | 25.05 | -1.58% |
| 27 | Math: Sinc | 27.82 | 26.12 | **6.11%** |
| 28 | Weird: Step-Slope | 21.64 | 21.79 | -0.69% |
| 29 | Gritty Bass | 54.60 | 54.62 | -0.04% |
| 30 | Hybrid Saw*Sine | 46.44 | 45.92 | **1.11%** |
| 31 | Razor Pulse | 65.50 | 64.19 | **2.00%** |
| 32 | Pulse 25% | 47.94 | 47.67 | **0.55%** |
| 33 | Pulse 75% | 47.66 | 47.45 | **0.45%** |
| 34 | Staircase 4 Step | 70.41 | 69.61 | **1.14%** |
| 35 | Bit Crush Bomb | 80.00 | 80.12 | -0.15% |
| 36 | Bit-Crushed Square | 62.22 | 63.11 | -1.44% |
| 37 | Pulse Train Wreck | 94.51 | 96.14 | -1.73% |
| 38 | Narrow | 38.94 | 38.53 | **1.04%** |
| 39 | Quantized Saw 8 | 37.41 | 36.67 | **1.99%** |
| 40 | PWM Synth (A=Width B=Sub) | 62.33 | 63.06 | -1.17% |
| 41 | PWM Gate | 41.45 | 38.15 | **7.96%** |
| 42 | Harmonic Switch | 68.51 | 64.62 | **5.68%** |
| 43 | Multi-Gate | 67.61 | 68.14 | -0.78% |
| 44 | Bitwise Staircase | 29.71 | 29.02 | **2.32%** |
| 45 | Bitwise XOR Wave | 62.54 | 62.80 | -0.41% |
| 46 | Hard Quantize Sine | 38.84 | 38.44 | **1.03%** |
| 47 | Comparator Fuzz | 40.26 | 41.17 | -2.26% |
| 48 | Warp Speed | 33.43 | 30.91 | **7.54%** |
| 49 | Ghost Wail | 46.33 | 46.73 | -0.87% |
| 50 | Laser Malfunction | 60.96 | 59.49 | **2.41%** |
| 51 | Hyperspace Glitch | 58.81 | 58.58 | **0.38%** |
| 52 | Shredded Saw | 54.74 | 55.14 | -0.73% |
| 53 | Glitch Sine | 69.49 | 68.71 | **1.12%** |
| 54 | 4-Segment Bump | 54.57 | 55.41 | -1.55% |
| 55 | Bird Call AM | 51.36 | 48.66 | **5.26%** |
| 56 | Phase Distortion (A=Amt B=Shape) | 72.51 | 66.18 | **8.73%** |
| 57 | Chaos Sine (A=ModRate B=ModAmt) | 47.31 | 47.75 | -0.92% |
| 58 | Phase Glitch | 49.23 | 50.56 | -2.71% |
| 59 | Phase Distortion Wave | 167.15 | 170.14 | -1.79% |
| 60 | PD: Resonant | 36.36 | 36.70 | -0.94% |
| 61 | PD: Wrap | 36.66 | 37.33 | -1.81% |
| 62 | PD: Spike | 29.36 | 30.09 | -2.49% |
| 63 | PD: Windowed | 33.95 | 34.67 | -2.12% |
| 64 | Classic FM EP (A=Index B=Detune) | 76.18 | 77.37 | -1.56% |
| 65 | FM Bass Growl (A=Fdbk B=Index) | 79.43 | 78.66 | **0.96%** |
| 66 | Freq Shifter FM (A=Shift B=Index) | 60.23 | 60.49 | -0.43% |
| 67 | Complex FM A=Index B=ModFreq | 56.74 | 51.98 | **8.40%** |
| 68 | FM Pluck | 140.46 | 143.71 | -2.31% |
| 69 | FM Pitched Grit | 104.05 | 105.91 | -1.78% |
| 70 | FM Dynamic Lead | 171.76 | 175.21 | -2.01% |
| 71 | FM Glassy Evolve | 99.71 | 102.06 | -2.35% |
| 72 | FM: Classic EP (A=Tine B=Bell C=Ratio) | 81.49 | 86.29 | -5.90% |
| 73 | FM: Growl Bass (A=Index B=Fdbk C=Ratio) | 73.55 | 75.73 | -2.97% |
| 74 | FM: Deep Sub | 32.23 | 32.45 | -0.68% |
| 75 | FM: Talker | 41.81 | 41.78 | **0.06%** |
| 76 | FM: Feedback Sim | 56.70 | 57.68 | -1.73% |
| 77 | FM: Cascaded | 62.61 | 62.64 | -0.05% |
| 78 | FM: Vowel-ish | 38.69 | 40.05 | -3.53% |
| 79 | FM: Sci-Fi Drone (A=Evolve B=Chaos C=Pitch) | 67.65 | 68.51 | -1.27% |
| 80 | FM Metallic Bell (A=Decay B=Ratio) | 95.53 | 92.67 | **2.99%** |
| 81 | FM Hollow Drone (A=ModMix B=ModRatio) | 91.75 | 88.77 | **3.25%** |
| 82 | FM Harsh Noise Sweep (A=Sweep B=Intensity) | 103.01 | 102.19 | **0.80%** |
| 83 | FM Soft Pad (A=Brightness B=Chorus) | 115.81 | 116.44 | -0.55% |
| 84 | FM Bipolar Sweep Pad | 83.81 | 84.96 | -1.37% |
| 85 | FM Clangorous Hit (A=Metal B=Dissonance) | 99.96 | 100.73 | -0.78% |
| 86 | FM: Noise | 59.26 | 59.07 | **0.32%** |
| 87 | FM Evolving SciFi (A=Evolve B=Harmonics) | 95.46 | 97.00 | -1.62% |
| 88 | FM: Metallic Bell (A=Decay B=Index2 C=Ratio) | 101.30 | 104.40 | -3.06% |
| 89 | FM: Glitchy Noise (A=Index B=Bit C=Rate) | 90.26 | 92.79 | -2.80% |
| 90 | FM: Metallic 1 | 40.35 | 40.64 | -0.72% |
| 91 | FM: Metallic 2 | 40.75 | 41.02 | -0.67% |
| 92 | Weird: AM Chaos | 33.95 | 34.38 | -1.27% |
| 93 | Sci-Fi Drone | 192.61 | 195.55 | -1.53% |
| 94 | Evolving Metallic Bell | 596.35 | 594.94 | **0.24%** |
| 95 | Alien Communication | 249.12 | 246.56 | **1.03%** |
| 96 | Sine Harmonics | 73.58 | 74.44 | -1.16% |
| 97 | Harmonic Noise Blast | 82.80 | 82.52 | **0.34%** |
| 98 | Brass | 112.56 | 117.20 | -4.12% |
| 99 | Bowed String | 127.25 | 123.09 | **3.27%** |
| 100 | Additive Square | 84.91 | 83.60 | **1.54%** |
| 101 | Electric Pianoish | 80.53 | 79.50 | **1.27%** |
| 102 | Classic Pad | 80.15 | 78.98 | **1.46%** |
| 103 | Additive Saw (A=Harms B=Shape) | 81.37 | 82.50 | -1.39% |
| 104 | Random Phase Additive (A=RndAmt B=Harm) | 124.32 | 123.27 | **0.84%** |
| 105 | Grit Additive (A=Grit B=Tone) | 138.64 | 135.52 | **2.25%** |
| 106 | Simple Minor Triad | 101.63 | 100.18 | **1.43%** |
| 107 | Add: Spec 1 | 64.31 | 62.13 | **3.39%** |
| 108 | Add: Spec 2 | 64.61 | 62.59 | **3.13%** |
| 109 | Add: Bell | 49.21 | 47.28 | **3.91%** |
| 110 | Add: Organ | 64.14 | 63.06 | **1.68%** |
| 111 | Add: Random Phase | 47.09 | 46.76 | **0.70%** |
| 112 | Formantish | 63.20 | 63.05 | **0.23%** |
| 113 | Vocal Ah | 99.55 | 100.17 | -0.62% |
| 114 | Reso Filter Sweep (A=Reso B=Cutoff) | 83.86 | 84.44 | -0.69% |
| 115 | Formant Vowel (A=Phsr1 B=Phsr2) | 523.76 | 465.10 | **11.20%** |
| 116 | Sync Sweep No Slant | 42.52 | 43.91 | -3.26% |
| 117 | Sync Sweep Cos Shape | 54.52 | 53.96 | **1.03%** |
| 118 | Smoothed Sync (A=SyncFreq B=Duty) | 68.42 | 68.57 | -0.22% |
| 119 | Limited Sync (A=SyncFreq B=Duty) | 57.26 | 56.18 | **1.89%** |
| 120 | Sync Sweep (A=SyncFreq B=Duty) | 57.31 | 56.25 | **1.86%** |
| 121 | Oooh Choir Formant | 1747.56 | 1783.93 | -2.08% |
| 122 | PD Vocal Formant | 28.32 | 28.61 | -1.02% |
| 123 | Sync Soft | 35.38 | 35.57 | -0.54% |
| 124 | Fractal Sine | 61.56 | 63.23 | -2.71% |
| 125 | FM Breathy Flute (A=Air B=PitchMod) | 93.97 | 103.91 | -10.58% |
| 126 | Add: Saw 8 | 273.96 | 271.96 | **0.73%** |
| 127 | Add: Square 8 | 286.29 | 282.28 | **1.40%** |
| 128 | Kick Drum | 61.51 | 62.32 | -1.32% |
| 129 | Snare Drum | 72.01 | 72.08 | -0.10% |
| 130 | Clap | 88.62 | 89.23 | -0.69% |
| 131 | Tom Drum | 61.65 | 62.41 | -1.24% |
| 132 | Cymbalish | 67.17 | 67.54 | -0.55% |
| 133 | Double Waves | 96.29 | 97.53 | -1.28% |
| 134 | Metal Impact | 69.11 | 70.00 | -1.28% |
| 135 | Bell Tone | 76.64 | 77.31 | -0.87% |
| 136 | Metallic Perc | 67.14 | 67.98 | -1.26% |
| 137 | Sigma Bell (A=Decay B=Metal) | 276.36 | 278.33 | -0.71% |
| 138 | Classic Noise Sim | 136.41 | 128.76 | **5.61%** |
| 139 | Distorted Pitch | 119.95 | 119.09 | **0.71%** |
| 140 | Gritty Rumble Noise | 110.84 | 144.80 | -30.63% |
| 141 | Filtered Static Noise | 437.78 | 527.36 | -20.46% |
| 142 | Wooden Percussion | 77.07 | 78.22 | -1.50% |
| 143 | Glitchy Percussion | 297.64 | 288.43 | **3.09%** |
| 144 | Plucked String (A=Damp B=Body) | 199.99 | 197.26 | **1.37%** |
| 145 | Sigma A=End B=Decay | 86.38 | 87.38 | -1.16% |
| 146 | Noisy Pad (A=NoiseAmt B=Flt) | 203.79 | 221.22 | -8.55% |
| 147 | Rich String Ensemble | 986.10 | 977.75 | **0.85%** |
| 148 | Mellow Brass Section | 529.01 | 534.13 | -0.97% |
| 149 | Jittery Inharmonic Pitch | 668.93 | 644.99 | **3.58%** |
| 150 | LFSR Granular Texture | 238.71 | 232.26 | **2.70%** |
| 151 | Morphing Harmonics | 1757.91 | 1840.27 | -4.69% |
| 152 | Breathing Pad | 139.19 | 136.74 | **1.76%** |
| 153 | Chaotic Oscillator | 169.52 | 168.76 | **0.45%** |
| 154 | Crystalline Arpeggio | 1061.17 | 1048.41 | **1.20%** |
| 155 | Add: Shepard Cycle | 42.92 | 42.69 | **0.55%** |
| 156 | Water Droplet | 69.29 | 68.79 | **0.72%** |
| 157 | Alien Chatter | 82.63 | 82.11 | **0.64%** |
| 158 | Weird: Chirp | 23.50 | 23.61 | -0.45% |
| 159 | Wind AM | 49.47 | 49.82 | -0.71% |
| 160 | LFSR Rhythm Gate | 109.23 | 102.93 | **5.77%** |
| 161 | LFSR Harmonic Chaos | 802.99 | 806.41 | -0.43% |
| 162 | LFSR Digital Texture | 204.75 | 207.11 | -1.15% |
| 163 | LFSR Poly Rhythm | 193.92 | 193.95 | -0.02% |
| 164 | LFSR Phase Modulation | 142.71 | 135.95 | **4.73%** |
| 165 | LFSR Granular | 557.86 | 552.87 | **0.90%** |
| 166 | LFSR Rhythmic Harmonics | 874.88 | 848.47 | **3.02%** |
| 167 | LFSR Spectral Shift | 203.21 | 203.00 | **0.10%** |
| 168 | LFSR Euclidean Beat | 136.91 | 139.33 | -1.77% |
| 169 | LFSR Feedback Synth | 165.44 | 161.62 | **2.31%** |
| 170 | LFSR Algorithmic Lead | 207.24 | 210.93 | -1.78% |
| 171 | LFSR Morphing Pad | 933.43 | 888.67 | **4.80%** |
| 172 | LFSR Breakbeat | 153.89 | 155.90 | -1.30% |
| 173 | LFSR Probability Gate | 156.20 | 154.98 | **0.78%** |
| 174 | LFSR Polyrhythmic Chaos | 185.90 | 184.19 | **0.92%** |
| 175 | LFSR Glitch Matrix | 569.43 | 575.99 | -1.15% |
| 176 | Pac-Man Wakka | 81.08 | 80.37 | **0.88%** |
| 177 | Pac-Man Power Pellet | 88.09 | 87.00 | **1.24%** |
| 178 | Pac-Man Death | 79.27 | 79.41 | -0.17% |
| 179 | Pac-Man Ghost | 84.61 | 81.56 | **3.61%** |
| 180 | Space Invaders Shot | 99.94 | 97.88 | **2.07%** |
| 181 | Space Invaders March | 68.88 | 68.54 | **0.49%** |
| 182 | Space Invaders UFO | 94.86 | 94.00 | **0.91%** |
| 183 | Space Invaders Explosion | 99.39 | 99.59 | -0.20% |
| 184 | Asteroids Thrust | 147.60 | 144.96 | **1.79%** |
| 185 | Asteroids Shoot | 106.10 | 103.13 | **2.80%** |
| 186 | Asteroids Explosion | 117.79 | 116.66 | **0.96%** |
| 187 | Asteroids Hyperspace | 119.52 | 117.56 | **1.64%** |
| 188 | Galaxian Attack | 77.93 | 78.91 | -1.25% |
| 189 | Galaxian Formation | 97.57 | 93.06 | **4.62%** |
| 190 | Centipede Laser | 115.95 | 112.00 | **3.41%** |
| 191 | Centipede Flea Drop | 95.22 | 87.38 | **8.24%** |
| 192 | Defender Thrust | 140.95 | 139.06 | **1.34%** |
| 193 | Defender Smart Bomb | 114.32 | 112.66 | **1.45%** |
| 194 | Frogger Hop | 100.27 | 98.19 | **2.07%** |
| 195 | Frogger Traffic | 105.60 | 104.91 | **0.65%** |
| 196 | Donkey Kong Hammer | 108.39 | 101.22 | **6.62%** |
| 197 | Donkey Kong Jump | 96.68 | 87.59 | **9.40%** |
| 198 | Missile Command Explosion | 118.16 | 117.83 | **0.28%** |
| 199 | Tempest Shoot | 110.07 | 107.17 | **2.63%** |
| 200 | Tempest Flip | 80.48 | 82.11 | -2.02% |
| 201 | Berzerk Robot Voice | 123.00 | 118.15 | **3.94%** |
| 202 | Robotron Shoot | 115.48 | 110.06 | **4.70%** |
| 203 | Phoenix Bird Cry | 99.17 | 94.36 | **4.85%** |
| 204 | Gorf Laser | 105.84 | 102.05 | **3.58%** |
| 205 | Scramble Engine | 142.30 | 140.38 | **1.35%** |
| 206 | Zaxxon Alarm | 96.02 | 97.06 | -1.08% |
| 207 | Moon Patrol Bounce | 118.83 | 119.54 | -0.60% |
| 208 | POKEY Pure Tone | 78.36 | 77.03 | **1.70%** |
| 209 | POKEY Filtered Noise | 120.49 | 111.10 | **7.79%** |
| 210 | POKEY Distorted Bass | 68.16 | 61.39 | **9.94%** |
| 211 | POKEY Laser Zap | 127.32 | 124.12 | **2.51%** |
| 212 | POKEY Explosion | 110.84 | 107.91 | **2.65%** |
| 213 | POKEY Engine Rumble | 138.82 | 137.88 | **0.68%** |
| 214 | POKEY Bit Crush Lead | 79.91 | 79.49 | **0.53%** |
| 215 | POKEY Coin Pickup | 90.51 | 91.08 | -0.62% |
| 216 | POKEY Jump Sound | 109.58 | 103.88 | **5.20%** |
| 217 | POKEY Chirp Bird | 133.68 | 127.31 | **4.77%** |
| 218 | POKEY Alien Voice | 149.94 | 145.59 | **2.90%** |
| 219 | POKEY Power Up | 99.92 | 98.16 | **1.76%** |
| 220 | POKEY Hit Sound | 106.99 | 104.02 | **2.78%** |
| 221 | POKEY Sweep Down | 90.01 | 86.67 | **3.71%** |
| 222 | POKEY Poly Counter | 111.27 | 109.56 | **1.54%** |
| 223 | POKEY Four Channel | 133.36 | 132.75 | **0.45%** |
| 224 | POKEY 4-bit Noise (64k) | 82.28 | 82.03 | **0.30%** |
| 225 | POKEY 5-bit Noise (64k) | 83.11 | 81.60 | **1.82%** |
| 226 | POKEY 17-bit Noise (64k) | 82.10 | 80.94 | **1.41%** |
| 227 | POKEY 9-bit Noise (15k) | 80.16 | 78.98 | **1.47%** |
| 228 | POKEY Filtered 4-bit (Fast) | 88.05 | 88.22 | -0.20% |
| 229 | POKEY Filtered 5-bit (Fast) | 88.26 | 88.09 | **0.19%** |
| 230 | POKEY Tone + 4-bit (64k) | 106.93 | 105.10 | **1.71%** |
| 231 | POKEY Tone + 5-bit (64k) | 107.79 | 106.23 | **1.44%** |
| 232 | POKEY Tone + 17-bit (64k) | 106.21 | 104.84 | **1.29%** |
| 233 | POKEY 4(64k)+5(15k) Combined | 161.12 | 164.97 | -2.39% |
| 234 | POKEY "High Pass" 4-bit (Fast) | 92.98 | 91.41 | **1.68%** |
| 235 | POKEY 64kHz Noise (17-bit) | 83.23 | 81.32 | **2.29%** |
| 236 | POKEY 15kHz Noise (9-bit) | 81.43 | 79.78 | **2.02%** |
| 237 | POKEY Engine Sound (Noise Gated) | 166.39 | 167.11 | -0.43% |
| 238 | POKEY Explosion (Decaying Rate/Vol) | 214.86 | 211.30 | **1.66%** |
| 239 | POKEY "Multi-Channel" (Mixed) | 244.00 | 245.28 | -0.52% |
| 240 | Logic: PWM Hash | 60.36 | 61.80 | -2.38% |
| 241 | Sample & Hold Sine | 39.59 | 38.92 | **1.69%** |
| 242 | Digital Saw | 28.54 | 27.19 | **4.73%** |
| 243 | Glitch Step | 25.58 | 25.73 | -0.61% |
| 244 | Weird: Gap | 25.79 | 26.01 | -0.83% |
| 245 | Noise: White-ish | 16.79 | 32.85 | -95.68% |
| 246 | Noise: S&H | 156.26 | 155.29 | **0.62%** |
| 247 | Fibonacci Series | 69.96 | 70.81 | -1.21% |
| 248 | Logistic Chaos | 64.01 | 65.64 | -2.54% |
| 249 | Chebyshev 4th | 70.95 | 71.39 | -0.62% |
| 250 | Tanh Fold | 59.87 | 61.91 | -3.40% |
| 251 | Exp FM | 52.32 | 52.71 | -0.75% |
| 252 | Chaotic Map | 59.00 | 61.17 | -3.67% |
| 253 | Pseudo-LPG | 75.47 | 75.97 | -0.66% |
| 254 | Harmonic Steps | 40.85 | 40.37 | **1.18%** |
| 255 | Vocal Formant 2 | 39.82 | 40.50 | -1.72% |

## Summary

* **Total Patches Benchmarked:** 256
* **Averaged Over:** 10 test runs
* **Average Time per Sample (v1.9.25):** 132.23 ns
* **Average Time per Sample (v1.9.26):** 131.99 ns
* **Overall Performance Improvement:** **0.18%**
