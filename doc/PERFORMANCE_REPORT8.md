# Polysonix VM Performance Report 8

This report compares the execution time of the new VM against the PERFORMANCE_REPORT7.md baseline.
Measurements are averaged over 10 runs to eliminate OS and cache noise.

| Patch ID | Name | Report 7 (Before) (ns) | Current (After) (ns) | Improvement |
| :--- | :--- | :--- | :--- | :--- |
| 0 | Triangle Up | 49.12 | 50.61 | -3.04% |
| 1 | Triangle Down | 49.92 | 50.75 | -1.65% |
| 2 | Sine Up | 94.72 | 90.60 | **4.35%** |
| 3 | Sine Down | 97.88 | 93.81 | **4.16%** |
| 4 | Square Up | 53.96 | 55.80 | -3.42% |
| 5 | Square Down | 54.61 | 56.02 | -2.57% |
| 6 | Saw Rising | 72.11 | 66.08 | **8.36%** |
| 7 | Saw Falling | 71.58 | 72.78 | -1.68% |
| 8 | Saw/Sine Up | 86.44 | 74.92 | **13.32%** |
| 9 | Sine/Saw Down | 91.57 | 79.39 | **13.31%** |
| 10 | Square/Sine Up | 77.69 | 68.25 | **12.14%** |
| 11 | Sine/Square Down | 78.38 | 67.84 | **13.45%** |
| 12 | Saw/Triangle Up | 39.80 | 40.15 | -0.88% |
| 13 | Triangle/Saw Down | 36.87 | 36.89 | -0.07% |
| 14 | Triangle/Sine Up | 66.72 | 56.91 | **14.71%** |
| 15 | Sine/Triangle Down | 67.01 | 59.04 | **11.89%** |
| 16 | Clipped Sine | 48.74 | 45.80 | **6.04%** |
| 17 | Rectified Sine | 48.05 | 48.98 | -1.94% |
| 18 | Sine * Saw | 54.51 | 53.73 | **1.44%** |
| 19 | Overload Spark | 71.13 | 73.69 | -3.61% |
| 20 | Overfolded Saw | 62.69 | 58.53 | **6.64%** |
| 21 | Clipped Chaos | 71.37 | 72.35 | -1.38% |
| 22 | Wavefolder Sim (A=Fold B=Bias) | 76.28 | 77.27 | -1.29% |
| 23 | Triangle Fold | 41.48 | 42.33 | -2.04% |
| 24 | Math: Tanh Drive | 33.53 | 34.24 | -2.12% |
| 25 | Math: Cubic | 32.11 | 33.78 | -5.22% |
| 26 | Math: Rectified | 24.79 | 25.46 | -2.70% |
| 27 | Math: Sinc | 25.90 | 27.34 | -5.56% |
| 28 | Weird: Step-Slope | 21.59 | 22.16 | -2.62% |
| 29 | Gritty Bass | 53.94 | 53.56 | **0.70%** |
| 30 | Hybrid Saw*Sine | 45.63 | 45.89 | -0.58% |
| 31 | Razor Pulse | 64.54 | 65.00 | -0.70% |
| 32 | Pulse 25% | 47.60 | 50.36 | -5.81% |
| 33 | Pulse 75% | 47.16 | 49.84 | -5.67% |
| 34 | Staircase 4 Step | 69.41 | 72.50 | -4.46% |
| 35 | Bit Crush Bomb | 79.56 | 82.78 | -4.05% |
| 36 | Bit-Crushed Square | 62.13 | 61.17 | **1.54%** |
| 37 | Pulse Train Wreck | 95.90 | 99.20 | -3.44% |
| 38 | Narrow | 38.11 | 39.33 | -3.20% |
| 39 | Quantized Saw 8 | 36.20 | 37.14 | -2.58% |
| 40 | PWM Synth (A=Width B=Sub) | 62.37 | 65.27 | -4.64% |
| 41 | PWM Gate | 38.05 | 38.78 | -1.92% |
| 42 | Harmonic Switch | 64.36 | 66.64 | -3.54% |
| 43 | Multi-Gate | 67.60 | 69.96 | -3.49% |
| 44 | Bitwise Staircase | 28.33 | 30.02 | -5.98% |
| 45 | Bitwise XOR Wave | 62.45 | 64.77 | -3.71% |
| 46 | Hard Quantize Sine | 37.97 | 39.48 | -3.98% |
| 47 | Comparator Fuzz | 40.70 | 41.29 | -1.45% |
| 48 | Warp Speed | 30.65 | 32.05 | -4.55% |
| 49 | Ghost Wail | 46.78 | 48.22 | -3.07% |
| 50 | Laser Malfunction | 58.98 | 56.45 | **4.28%** |
| 51 | Hyperspace Glitch | 58.55 | 62.53 | -6.79% |
| 52 | Shredded Saw | 54.58 | 54.75 | -0.31% |
| 53 | Glitch Sine | 67.78 | 69.79 | -2.97% |
| 54 | 4-Segment Bump | 54.97 | 55.49 | -0.96% |
| 55 | Bird Call AM | 48.34 | 49.61 | -2.63% |
| 56 | Phase Distortion (A=Amt B=Shape) | 65.77 | 66.69 | -1.41% |
| 57 | Chaos Sine (A=ModRate B=ModAmt) | 47.11 | 48.36 | -2.65% |
| 58 | Phase Glitch | 49.83 | 51.28 | -2.92% |
| 59 | Phase Distortion Wave | 168.00 | 172.29 | -2.55% |
| 60 | PD: Resonant | 36.38 | 37.66 | -3.52% |
| 61 | PD: Wrap | 37.02 | 38.50 | -4.01% |
| 62 | PD: Spike | 30.05 | 31.41 | -4.51% |
| 63 | PD: Windowed | 34.31 | 35.47 | -3.38% |
| 64 | Classic FM EP (A=Index B=Detune) | 76.37 | 79.12 | -3.61% |
| 65 | FM Bass Growl (A=Fdbk B=Index) | 77.94 | 83.71 | -7.40% |
| 66 | Freq Shifter FM (A=Shift B=Index) | 59.98 | 62.09 | -3.52% |
| 67 | Complex FM A=Index B=ModFreq | 51.69 | 53.20 | -2.92% |
| 68 | FM Pluck | 141.39 | 148.11 | -4.75% |
| 69 | FM Pitched Grit | 104.63 | 106.56 | -1.84% |
| 70 | FM Dynamic Lead | 172.84 | 175.44 | -1.51% |
| 71 | FM Glassy Evolve | 101.64 | 104.03 | -2.36% |
| 72 | FM: Classic EP (A=Tine B=Bell C=Ratio) | 81.69 | 84.50 | -3.45% |
| 73 | FM: Growl Bass (A=Index B=Fdbk C=Ratio) | 73.81 | 75.37 | -2.11% |
| 74 | FM: Deep Sub | 32.37 | 33.38 | -3.14% |
| 75 | FM: Talker | 41.25 | 43.59 | -5.68% |
| 76 | FM: Feedback Sim | 57.43 | 58.94 | -2.63% |
| 77 | FM: Cascaded | 62.61 | 63.69 | -1.72% |
| 78 | FM: Vowel-ish | 39.92 | 40.91 | -2.49% |
| 79 | FM: Sci-Fi Drone (A=Evolve B=Chaos C=Pitch) | 68.15 | 69.60 | -2.13% |
| 80 | FM Metallic Bell (A=Decay B=Ratio) | 92.33 | 98.03 | -6.17% |
| 81 | FM Hollow Drone (A=ModMix B=ModRatio) | 88.13 | 85.91 | **2.51%** |
| 82 | FM Harsh Noise Sweep (A=Sweep B=Intensity) | 101.23 | 108.77 | -7.45% |
| 83 | FM Soft Pad (A=Brightness B=Chorus) | 116.03 | 121.64 | -4.83% |
| 84 | FM Bipolar Sweep Pad | 83.62 | 88.40 | -5.72% |
| 85 | FM Clangorous Hit (A=Metal B=Dissonance) | 100.47 | 102.13 | -1.66% |
| 86 | FM: Noise | 58.86 | 60.92 | -3.49% |
| 87 | FM Evolving SciFi (A=Evolve B=Harmonics) | 96.12 | 99.36 | -3.37% |
| 88 | FM: Metallic Bell (A=Decay B=Index2 C=Ratio) | 103.87 | 104.42 | -0.53% |
| 89 | FM: Glitchy Noise (A=Index B=Bit C=Rate) | 92.35 | 95.27 | -3.16% |
| 90 | FM: Metallic 1 | 40.39 | 41.76 | -3.39% |
| 91 | FM: Metallic 2 | 40.75 | 42.27 | -3.72% |
| 92 | Weird: AM Chaos | 34.33 | 35.73 | -4.09% |
| 93 | Sci-Fi Drone | 192.88 | 195.49 | -1.35% |
| 94 | Evolving Metallic Bell | 595.18 | 628.82 | -5.65% |
| 95 | Alien Communication | 245.44 | 256.70 | -4.59% |
| 96 | Sine Harmonics | 74.20 | 76.07 | -2.52% |
| 97 | Harmonic Noise Blast | 82.23 | 85.03 | -3.41% |
| 98 | Brass | 115.87 | 118.45 | -2.23% |
| 99 | Bowed String | 122.79 | 130.88 | -6.58% |
| 100 | Additive Square | 83.28 | 84.91 | -1.96% |
| 101 | Electric Pianoish | 79.00 | 80.45 | -1.84% |
| 102 | Classic Pad | 78.64 | 81.09 | -3.12% |
| 103 | Additive Saw (A=Harms B=Shape) | 82.17 | 83.88 | -2.08% |
| 104 | Random Phase Additive (A=RndAmt B=Harm) | 122.29 | 129.12 | -5.59% |
| 105 | Grit Additive (A=Grit B=Tone) | 133.88 | 137.80 | -2.92% |
| 106 | Simple Minor Triad | 100.12 | 103.45 | -3.33% |
| 107 | Add: Spec 1 | 61.47 | 63.51 | -3.31% |
| 108 | Add: Spec 2 | 62.02 | 63.37 | -2.18% |
| 109 | Add: Bell | 46.95 | 48.76 | -3.86% |
| 110 | Add: Organ | 62.18 | 63.34 | -1.87% |
| 111 | Add: Random Phase | 46.40 | 48.33 | -4.15% |
| 112 | Formantish | 62.08 | 64.10 | -3.25% |
| 113 | Vocal Ah | 99.65 | 102.00 | -2.35% |
| 114 | Reso Filter Sweep (A=Reso B=Cutoff) | 83.95 | 84.89 | -1.13% |
| 115 | Formant Vowel (A=Phsr1 B=Phsr2) | 452.50 | 478.51 | -5.75% |
| 116 | Sync Sweep No Slant | 43.58 | 41.06 | **5.78%** |
| 117 | Sync Sweep Cos Shape | 53.88 | 51.69 | **4.06%** |
| 118 | Smoothed Sync (A=SyncFreq B=Duty) | 67.56 | 69.03 | -2.17% |
| 119 | Limited Sync (A=SyncFreq B=Duty) | 55.92 | 52.73 | **5.71%** |
| 120 | Sync Sweep (A=SyncFreq B=Duty) | 55.80 | 52.82 | **5.34%** |
| 121 | Oooh Choir Formant | 1766.28 | 1833.70 | -3.82% |
| 122 | PD Vocal Formant | 28.38 | 29.71 | -4.70% |
| 123 | Sync Soft | 35.42 | 41.55 | -17.29% |
| 124 | Fractal Sine | 62.98 | 63.48 | -0.79% |
| 125 | FM Breathy Flute (A=Air B=PitchMod) | 102.80 | 105.94 | -3.05% |
| 126 | Add: Saw 8 | 268.90 | 279.04 | -3.77% |
| 127 | Add: Square 8 | 280.13 | 287.58 | -2.66% |
| 128 | Kick Drum | 61.94 | 63.88 | -3.13% |
| 129 | Snare Drum | 71.36 | 73.20 | -2.59% |
| 130 | Clap | 88.56 | 89.65 | -1.23% |
| 131 | Tom Drum | 62.04 | 64.78 | -4.42% |
| 132 | Cymbalish | 67.64 | 70.41 | -4.10% |
| 133 | Double Waves | 97.72 | 113.01 | -15.65% |
| 134 | Metal Impact | 69.56 | 71.55 | -2.85% |
| 135 | Bell Tone | 77.16 | 80.28 | -4.05% |
| 136 | Metallic Perc | 67.55 | 69.19 | -2.43% |
| 137 | Sigma Bell (A=Decay B=Metal) | 274.71 | 293.78 | -6.94% |
| 138 | Classic Noise Sim | 132.46 | 137.03 | -3.45% |
| 139 | Distorted Pitch | 117.86 | 124.55 | -5.68% |
| 140 | Gritty Rumble Noise | 144.86 | 152.76 | -5.46% |
| 141 | Filtered Static Noise | 532.75 | 536.82 | -0.76% |
| 142 | Wooden Percussion | 78.16 | 80.64 | -3.17% |
| 143 | Glitchy Percussion | 289.13 | 302.93 | -4.77% |
| 144 | Plucked String (A=Damp B=Body) | 194.98 | 205.93 | -5.62% |
| 145 | Sigma A=End B=Decay | 86.51 | 87.93 | -1.64% |
| 146 | Noisy Pad (A=NoiseAmt B=Flt) | 219.49 | 218.10 | **0.63%** |
| 147 | Rich String Ensemble | 961.76 | 1016.19 | -5.66% |
| 148 | Mellow Brass Section | 533.79 | 556.00 | -4.16% |
| 149 | Jittery Inharmonic Pitch | 618.60 | 637.03 | -2.98% |
| 150 | LFSR Granular Texture | 230.63 | 242.77 | -5.27% |
| 151 | Morphing Harmonics | 1833.00 | 1941.66 | -5.93% |
| 152 | Breathing Pad | 136.78 | 140.68 | -2.85% |
| 153 | Chaotic Oscillator | 167.19 | 169.03 | -1.10% |
| 154 | Crystalline Arpeggio | 1044.03 | 1191.50 | -14.13% |
| 155 | Add: Shepard Cycle | 42.52 | 44.33 | -4.26% |
| 156 | Water Droplet | 68.79 | 71.21 | -3.51% |
| 157 | Alien Chatter | 82.03 | 92.84 | -13.18% |
| 158 | Weird: Chirp | 23.43 | 24.28 | -3.63% |
| 159 | Wind AM | 49.25 | 50.67 | -2.88% |
| 160 | LFSR Rhythm Gate | 101.86 | 107.15 | -5.19% |
| 161 | LFSR Harmonic Chaos | 799.12 | 814.69 | -1.95% |
| 162 | LFSR Digital Texture | 205.40 | 217.08 | -5.69% |
| 163 | LFSR Poly Rhythm | 192.91 | 217.87 | -12.94% |
| 164 | LFSR Phase Modulation | 134.94 | 134.93 | **0.01%** |
| 165 | LFSR Granular | 546.04 | 592.14 | -8.44% |
| 166 | LFSR Rhythmic Harmonics | 834.32 | 949.10 | -13.76% |
| 167 | LFSR Spectral Shift | 201.70 | 206.18 | -2.22% |
| 168 | LFSR Euclidean Beat | 137.03 | 157.60 | -15.01% |
| 169 | LFSR Feedback Synth | 160.76 | 157.76 | **1.86%** |
| 170 | LFSR Algorithmic Lead | 209.75 | 222.28 | -5.98% |
| 171 | LFSR Morphing Pad | 882.96 | 923.34 | -4.57% |
| 172 | LFSR Breakbeat | 153.93 | 172.98 | -12.38% |
| 173 | LFSR Probability Gate | 154.04 | 157.58 | -2.30% |
| 174 | LFSR Polyrhythmic Chaos | 182.84 | 215.45 | -17.84% |
| 175 | LFSR Glitch Matrix | 567.89 | 599.74 | -5.61% |
| 176 | Pac-Man Wakka | 80.16 | 82.30 | -2.67% |
| 177 | Pac-Man Power Pellet | 86.70 | 88.84 | -2.47% |
| 178 | Pac-Man Death | 78.75 | 81.22 | -3.13% |
| 179 | Pac-Man Ghost | 81.33 | 84.03 | -3.31% |
| 180 | Space Invaders Shot | 97.75 | 99.34 | -1.63% |
| 181 | Space Invaders March | 67.56 | 71.16 | -5.33% |
| 182 | Space Invaders UFO | 92.67 | 95.67 | -3.24% |
| 183 | Space Invaders Explosion | 98.00 | 100.30 | -2.34% |
| 184 | Asteroids Thrust | 142.93 | 148.40 | -3.83% |
| 185 | Asteroids Shoot | 102.49 | 108.09 | -5.46% |
| 186 | Asteroids Explosion | 115.31 | 117.83 | -2.19% |
| 187 | Asteroids Hyperspace | 116.70 | 119.44 | -2.35% |
| 188 | Galaxian Attack | 77.61 | 79.39 | -2.30% |
| 189 | Galaxian Formation | 92.37 | 95.71 | -3.62% |
| 190 | Centipede Laser | 110.64 | 113.88 | -2.93% |
| 191 | Centipede Flea Drop | 86.88 | 91.89 | -5.76% |
| 192 | Defender Thrust | 137.75 | 139.28 | -1.11% |
| 193 | Defender Smart Bomb | 112.88 | 115.56 | -2.37% |
| 194 | Frogger Hop | 98.19 | 99.81 | -1.65% |
| 195 | Frogger Traffic | 104.59 | 105.75 | -1.11% |
| 196 | Donkey Kong Hammer | 100.32 | 104.34 | -4.01% |
| 197 | Donkey Kong Jump | 87.50 | 89.90 | -2.74% |
| 198 | Missile Command Explosion | 115.88 | 118.58 | -2.33% |
| 199 | Tempest Shoot | 107.15 | 112.12 | -4.63% |
| 200 | Tempest Flip | 81.10 | 82.22 | -1.37% |
| 201 | Berzerk Robot Voice | 118.87 | 121.85 | -2.51% |
| 202 | Robotron Shoot | 109.78 | 112.25 | -2.25% |
| 203 | Phoenix Bird Cry | 94.06 | 96.72 | -2.82% |
| 204 | Gorf Laser | 101.27 | 103.98 | -2.68% |
| 205 | Scramble Engine | 139.50 | 140.57 | -0.77% |
| 206 | Zaxxon Alarm | 96.18 | 99.75 | -3.72% |
| 207 | Moon Patrol Bounce | 119.07 | 123.72 | -3.91% |
| 208 | POKEY Pure Tone | 76.32 | 79.83 | -4.60% |
| 209 | POKEY Filtered Noise | 110.02 | 117.08 | -6.41% |
| 210 | POKEY Distorted Bass | 61.52 | 65.25 | -6.07% |
| 211 | POKEY Laser Zap | 123.31 | 124.56 | -1.02% |
| 212 | POKEY Explosion | 106.82 | 109.66 | -2.66% |
| 213 | POKEY Engine Rumble | 135.21 | 139.51 | -3.18% |
| 214 | POKEY Bit Crush Lead | 78.99 | 82.86 | -4.90% |
| 215 | POKEY Coin Pickup | 91.00 | 94.93 | -4.32% |
| 216 | POKEY Jump Sound | 103.15 | 106.31 | -3.07% |
| 217 | POKEY Chirp Bird | 127.55 | 128.53 | -0.77% |
| 218 | POKEY Alien Voice | 145.42 | 148.62 | -2.20% |
| 219 | POKEY Power Up | 98.56 | 100.77 | -2.24% |
| 220 | POKEY Hit Sound | 103.25 | 107.80 | -4.40% |
| 221 | POKEY Sweep Down | 85.94 | 87.97 | -2.37% |
| 222 | POKEY Poly Counter | 109.58 | 113.41 | -3.49% |
| 223 | POKEY Four Channel | 133.02 | 136.08 | -2.30% |
| 224 | POKEY 4-bit Noise (64k) | 81.34 | 80.98 | **0.44%** |
| 225 | POKEY 5-bit Noise (64k) | 80.40 | 83.91 | -4.37% |
| 226 | POKEY 17-bit Noise (64k) | 79.52 | 83.51 | -5.02% |
| 227 | POKEY 9-bit Noise (15k) | 77.94 | 80.32 | -3.05% |
| 228 | POKEY Filtered 4-bit (Fast) | 87.14 | 87.45 | -0.36% |
| 229 | POKEY Filtered 5-bit (Fast) | 87.69 | 87.91 | -0.25% |
| 230 | POKEY Tone + 4-bit (64k) | 104.34 | 108.35 | -3.85% |
| 231 | POKEY Tone + 5-bit (64k) | 105.16 | 109.67 | -4.29% |
| 232 | POKEY Tone + 17-bit (64k) | 104.06 | 108.66 | -4.42% |
| 233 | POKEY 4(64k)+5(15k) Combined | 164.73 | 171.56 | -4.15% |
| 234 | POKEY "High Pass" 4-bit (Fast) | 90.87 | 93.19 | -2.55% |
| 235 | POKEY 64kHz Noise (17-bit) | 80.78 | 82.61 | -2.26% |
| 236 | POKEY 15kHz Noise (9-bit) | 79.18 | 81.06 | -2.37% |
| 237 | POKEY Engine Sound (Noise Gated) | 165.12 | 167.24 | -1.28% |
| 238 | POKEY Explosion (Decaying Rate/Vol) | 209.34 | 220.23 | -5.20% |
| 239 | POKEY "Multi-Channel" (Mixed) | 242.36 | 244.65 | -0.94% |
| 240 | Logic: PWM Hash | 61.11 | 36.06 | **40.98%** |
| 241 | Sample & Hold Sine | 38.73 | 39.55 | -2.13% |
| 242 | Digital Saw | 26.78 | 27.24 | -1.72% |
| 243 | Glitch Step | 25.65 | 28.27 | -10.23% |
| 244 | Weird: Gap | 25.73 | 25.99 | -1.03% |
| 245 | Noise: White-ish | 32.45 | 33.01 | -1.71% |
| 246 | Noise: S&H | 153.74 | 156.75 | -1.95% |
| 247 | Fibonacci Series | 69.95 | 72.25 | -3.28% |
| 248 | Logistic Chaos | 64.83 | 65.90 | -1.65% |
| 249 | Chebyshev 4th | 70.80 | 71.85 | -1.48% |
| 250 | Tanh Fold | 61.23 | 61.53 | -0.49% |
| 251 | Exp FM | 52.30 | 53.04 | -1.41% |
| 252 | Chaotic Map | 60.65 | 61.80 | -1.89% |
| 253 | Pseudo-LPG | 75.70 | 77.33 | -2.15% |
| 254 | Harmonic Steps | 40.37 | 41.28 | -2.25% |
| 255 | Vocal Formant 2 | 40.42 | 41.18 | -1.88% |

## Summary

* **Total Patches Benchmarked:** 256
* **Averaged Over:** 10 test runs
* **Average Time per Sample (Report 7):** 130.88 ns
* **Average Time per Sample (Current):** 136.07 ns
* **Overall Performance Improvement:** **-3.97%**
