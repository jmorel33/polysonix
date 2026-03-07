# Polysonix VM v1.9.26 (FMA) Performance Report

This report compares the execution time of the new Flat Opcode VM (v1.9.26 (FMA)) against the v1.9.25 baseline.
Both sets of measurements were taken on this cloud environment for direct comparison.

| Patch ID | Name | v1.9.25 (Before) (ns) | v1.9.26 (After) (ns) | Improvement |
| :--- | :--- | :--- | :--- | :--- |
| 0 | Triangle Up | 49.74 | 49.92 | -0.36% |
| 1 | Triangle Down | 51.25 | 50.36 | **1.74%** |
| 2 | Sine Up | 95.16 | 96.45 | -1.36% |
| 3 | Sine Down | 97.31 | 99.95 | -2.71% |
| 4 | Square Up | 55.58 | 54.02 | **2.81%** |
| 5 | Square Down | 54.49 | 54.13 | **0.66%** |
| 6 | Saw Rising | 71.56 | 71.59 | -0.04% |
| 7 | Saw Falling | 71.97 | 71.68 | **0.40%** |
| 8 | Saw/Sine Up | 87.27 | 87.26 | **0.01%** |
| 9 | Sine/Saw Down | 92.33 | 92.08 | **0.27%** |
| 10 | Square/Sine Up | 79.21 | 78.54 | **0.85%** |
| 11 | Sine/Square Down | 79.48 | 79.34 | **0.18%** |
| 12 | Saw/Triangle Up | 44.10 | 39.74 | **9.89%** |
| 13 | Triangle/Saw Down | 37.70 | 37.28 | **1.11%** |
| 14 | Triangle/Sine Up | 68.42 | 68.62 | -0.29% |
| 15 | Sine/Triangle Down | 71.42 | 69.00 | **3.39%** |
| 16 | Clipped Sine | 48.79 | 48.42 | **0.76%** |
| 17 | Rectified Sine | 48.59 | 49.65 | -2.18% |
| 18 | Sine * Saw | 53.82 | 53.91 | -0.17% |
| 19 | Overload Spark | 74.64 | 70.62 | **5.39%** |
| 20 | Overfolded Saw | 63.22 | 62.97 | **0.40%** |
| 21 | Clipped Chaos | 73.35 | 71.98 | **1.87%** |
| 22 | Wavefolder Sim (A=Fold B=Bias) | 77.60 | 76.07 | **1.97%** |
| 23 | Triangle Fold | 41.21 | 41.35 | -0.34% |
| 24 | Math: Tanh Drive | 33.71 | 33.62 | **0.27%** |
| 25 | Math: Cubic | 32.25 | 32.15 | **0.31%** |
| 26 | Math: Rectified | 24.66 | 25.17 | -2.07% |
| 27 | Math: Sinc | 27.82 | 26.23 | **5.72%** |
| 28 | Weird: Step-Slope | 21.64 | 21.81 | -0.79% |
| 29 | Gritty Bass | 54.60 | 56.33 | -3.17% |
| 30 | Hybrid Saw*Sine | 46.44 | 45.70 | **1.59%** |
| 31 | Razor Pulse | 65.50 | 65.63 | -0.20% |
| 32 | Pulse 25% | 47.94 | 47.38 | **1.17%** |
| 33 | Pulse 75% | 47.66 | 47.74 | -0.17% |
| 34 | Staircase 4 Step | 70.41 | 69.64 | **1.09%** |
| 35 | Bit Crush Bomb | 80.00 | 79.88 | **0.15%** |
| 36 | Bit-Crushed Square | 62.22 | 62.09 | **0.21%** |
| 37 | Pulse Train Wreck | 94.51 | 93.87 | **0.68%** |
| 38 | Narrow | 38.94 | 38.14 | **2.05%** |
| 39 | Quantized Saw 8 | 37.41 | 36.20 | **3.23%** |
| 40 | PWM Synth (A=Width B=Sub) | 62.33 | 63.61 | -2.05% |
| 41 | PWM Gate | 41.45 | 38.17 | **7.91%** |
| 42 | Harmonic Switch | 68.51 | 63.74 | **6.96%** |
| 43 | Multi-Gate | 67.61 | 68.11 | -0.74% |
| 44 | Bitwise Staircase | 29.71 | 28.51 | **4.04%** |
| 45 | Bitwise XOR Wave | 62.54 | 62.42 | **0.19%** |
| 46 | Hard Quantize Sine | 38.84 | 38.37 | **1.21%** |
| 47 | Comparator Fuzz | 40.26 | 41.19 | -2.31% |
| 48 | Warp Speed | 33.43 | 30.81 | **7.84%** |
| 49 | Ghost Wail | 46.33 | 46.43 | -0.22% |
| 50 | Laser Malfunction | 60.96 | 58.90 | **3.38%** |
| 51 | Hyperspace Glitch | 58.81 | 58.47 | **0.58%** |
| 52 | Shredded Saw | 54.74 | 56.76 | -3.69% |
| 53 | Glitch Sine | 69.49 | 67.39 | **3.02%** |
| 54 | 4-Segment Bump | 54.57 | 55.01 | -0.81% |
| 55 | Bird Call AM | 51.36 | 48.23 | **6.09%** |
| 56 | Phase Distortion (A=Amt B=Shape) | 72.51 | 65.16 | **10.14%** |
| 57 | Chaos Sine (A=ModRate B=ModAmt) | 47.31 | 47.08 | **0.49%** |
| 58 | Phase Glitch | 49.23 | 49.75 | -1.06% |
| 59 | Phase Distortion Wave | 167.15 | 168.42 | -0.76% |
| 60 | PD: Resonant | 36.36 | 36.36 | 0.00% |
| 61 | PD: Wrap | 36.66 | 37.03 | -1.01% |
| 62 | PD: Spike | 29.36 | 29.76 | -1.36% |
| 63 | PD: Windowed | 33.95 | 34.58 | -1.86% |
| 64 | Classic FM EP (A=Index B=Detune) | 76.18 | 76.25 | -0.09% |
| 65 | FM Bass Growl (A=Fdbk B=Index) | 79.43 | 78.20 | **1.55%** |
| 66 | Freq Shifter FM (A=Shift B=Index) | 60.23 | 60.26 | -0.05% |
| 67 | Complex FM A=Index B=ModFreq | 56.74 | 51.84 | **8.64%** |
| 68 | FM Pluck | 140.46 | 143.75 | -2.34% |
| 69 | FM Pitched Grit | 104.05 | 105.74 | -1.62% |
| 70 | FM Dynamic Lead | 171.76 | 173.01 | -0.73% |
| 71 | FM Glassy Evolve | 99.71 | 102.19 | -2.49% |
| 72 | FM: Classic EP (A=Tine B=Bell C=Ratio) | 81.49 | 82.63 | -1.40% |
| 73 | FM: Growl Bass (A=Index B=Fdbk C=Ratio) | 73.55 | 74.93 | -1.88% |
| 74 | FM: Deep Sub | 32.23 | 32.60 | -1.15% |
| 75 | FM: Talker | 41.81 | 41.93 | -0.29% |
| 76 | FM: Feedback Sim | 56.70 | 57.96 | -2.22% |
| 77 | FM: Cascaded | 62.61 | 62.28 | **0.53%** |
| 78 | FM: Vowel-ish | 38.69 | 40.07 | -3.57% |
| 79 | FM: Sci-Fi Drone (A=Evolve B=Chaos C=Pitch) | 67.65 | 68.23 | -0.86% |
| 80 | FM Metallic Bell (A=Decay B=Ratio) | 95.53 | 96.95 | -1.49% |
| 81 | FM Hollow Drone (A=ModMix B=ModRatio) | 91.75 | 88.71 | **3.31%** |
| 82 | FM Harsh Noise Sweep (A=Sweep B=Intensity) | 103.01 | 104.08 | -1.04% |
| 83 | FM Soft Pad (A=Brightness B=Chorus) | 115.81 | 119.85 | -3.49% |
| 84 | FM Bipolar Sweep Pad | 83.81 | 83.74 | **0.08%** |
| 85 | FM Clangorous Hit (A=Metal B=Dissonance) | 99.96 | 109.31 | -9.35% |
| 86 | FM: Noise | 59.26 | 62.72 | -5.84% |
| 87 | FM Evolving SciFi (A=Evolve B=Harmonics) | 95.46 | 109.39 | -14.59% |
| 88 | FM: Metallic Bell (A=Decay B=Index2 C=Ratio) | 101.30 | 105.19 | -3.84% |
| 89 | FM: Glitchy Noise (A=Index B=Bit C=Rate) | 90.26 | 107.21 | -18.78% |
| 90 | FM: Metallic 1 | 40.35 | 40.99 | -1.59% |
| 91 | FM: Metallic 2 | 40.75 | 42.12 | -3.36% |
| 92 | Weird: AM Chaos | 33.95 | 35.57 | -4.77% |
| 93 | Sci-Fi Drone | 192.61 | 198.25 | -2.93% |
| 94 | Evolving Metallic Bell | 596.35 | 651.82 | -9.30% |
| 95 | Alien Communication | 249.12 | 249.27 | -0.06% |
| 96 | Sine Harmonics | 73.58 | 76.55 | -4.04% |
| 97 | Harmonic Noise Blast | 82.80 | 84.95 | -2.60% |
| 98 | Brass | 112.56 | 119.75 | -6.39% |
| 99 | Bowed String | 127.25 | 126.98 | **0.21%** |
| 100 | Additive Square | 84.91 | 85.25 | -0.40% |
| 101 | Electric Pianoish | 80.53 | 83.60 | -3.81% |
| 102 | Classic Pad | 80.15 | 84.71 | -5.69% |
| 103 | Additive Saw (A=Harms B=Shape) | 81.37 | 82.29 | -1.13% |
| 104 | Random Phase Additive (A=RndAmt B=Harm) | 124.32 | 141.21 | -13.59% |
| 105 | Grit Additive (A=Grit B=Tone) | 138.64 | 205.88 | -48.50% |
| 106 | Simple Minor Triad | 101.63 | 100.84 | **0.78%** |
| 107 | Add: Spec 1 | 64.31 | 73.61 | -14.46% |
| 108 | Add: Spec 2 | 64.61 | 77.51 | -19.97% |
| 109 | Add: Bell | 49.21 | 57.35 | -16.54% |
| 110 | Add: Organ | 64.14 | 113.78 | -77.39% |
| 111 | Add: Random Phase | 47.09 | 67.33 | -42.98% |
| 112 | Formantish | 63.20 | 77.44 | -22.53% |
| 113 | Vocal Ah | 99.55 | 100.83 | -1.29% |
| 114 | Reso Filter Sweep (A=Reso B=Cutoff) | 83.86 | 91.40 | -8.99% |
| 115 | Formant Vowel (A=Phsr1 B=Phsr2) | 523.76 | 611.33 | -16.72% |
| 116 | Sync Sweep No Slant | 42.52 | 44.12 | -3.76% |
| 117 | Sync Sweep Cos Shape | 54.52 | 54.36 | **0.29%** |
| 118 | Smoothed Sync (A=SyncFreq B=Duty) | 68.42 | 69.11 | -1.01% |
| 119 | Limited Sync (A=SyncFreq B=Duty) | 57.26 | 56.33 | **1.62%** |
| 120 | Sync Sweep (A=SyncFreq B=Duty) | 57.31 | 56.14 | **2.04%** |
| 121 | Oooh Choir Formant | 1747.56 | 1781.19 | -1.92% |
| 122 | PD Vocal Formant | 28.32 | 28.85 | -1.87% |
| 123 | Sync Soft | 35.38 | 35.62 | -0.68% |
| 124 | Fractal Sine | 61.56 | 63.37 | -2.94% |
| 125 | FM Breathy Flute (A=Air B=PitchMod) | 93.97 | 103.73 | -10.39% |
| 126 | Add: Saw 8 | 273.96 | 270.87 | **1.13%** |
| 127 | Add: Square 8 | 286.29 | 282.64 | **1.27%** |
| 128 | Kick Drum | 61.51 | 62.42 | -1.48% |
| 129 | Snare Drum | 72.01 | 72.98 | -1.35% |
| 130 | Clap | 88.62 | 90.41 | -2.02% |
| 131 | Tom Drum | 61.65 | 62.32 | -1.09% |
| 132 | Cymbalish | 67.17 | 68.10 | -1.38% |
| 133 | Double Waves | 96.29 | 99.70 | -3.54% |
| 134 | Metal Impact | 69.11 | 69.96 | -1.23% |
| 135 | Bell Tone | 76.64 | 77.57 | -1.21% |
| 136 | Metallic Perc | 67.14 | 67.96 | -1.22% |
| 137 | Sigma Bell (A=Decay B=Metal) | 276.36 | 288.49 | -4.39% |
| 138 | Classic Noise Sim | 136.41 | 129.69 | **4.93%** |
| 139 | Distorted Pitch | 119.95 | 119.34 | **0.51%** |
| 140 | Gritty Rumble Noise | 110.84 | 145.72 | -31.47% |
| 141 | Filtered Static Noise | 437.78 | 528.14 | -20.64% |
| 142 | Wooden Percussion | 77.07 | 78.00 | -1.21% |
| 143 | Glitchy Percussion | 297.64 | 289.56 | **2.71%** |
| 144 | Plucked String (A=Damp B=Body) | 199.99 | 196.16 | **1.92%** |
| 145 | Sigma A=End B=Decay | 86.38 | 87.04 | -0.76% |
| 146 | Noisy Pad (A=NoiseAmt B=Flt) | 203.79 | 222.00 | -8.94% |
| 147 | Rich String Ensemble | 986.10 | 979.76 | **0.64%** |
| 148 | Mellow Brass Section | 529.01 | 534.69 | -1.07% |
| 149 | Jittery Inharmonic Pitch | 668.93 | 626.56 | **6.33%** |
| 150 | LFSR Granular Texture | 238.71 | 233.51 | **2.18%** |
| 151 | Morphing Harmonics | 1757.91 | 1860.38 | -5.83% |
| 152 | Breathing Pad | 139.19 | 136.65 | **1.82%** |
| 153 | Chaotic Oscillator | 169.52 | 168.74 | **0.46%** |
| 154 | Crystalline Arpeggio | 1061.17 | 1048.10 | **1.23%** |
| 155 | Add: Shepard Cycle | 42.92 | 42.68 | **0.56%** |
| 156 | Water Droplet | 69.29 | 70.18 | -1.28% |
| 157 | Alien Chatter | 82.63 | 82.61 | **0.02%** |
| 158 | Weird: Chirp | 23.50 | 23.94 | -1.87% |
| 159 | Wind AM | 49.47 | 49.35 | **0.24%** |
| 160 | LFSR Rhythm Gate | 109.23 | 102.48 | **6.18%** |
| 161 | LFSR Harmonic Chaos | 802.99 | 803.56 | -0.07% |
| 162 | LFSR Digital Texture | 204.75 | 208.54 | -1.85% |
| 163 | LFSR Poly Rhythm | 193.92 | 193.99 | -0.04% |
| 164 | LFSR Phase Modulation | 142.71 | 135.99 | **4.71%** |
| 165 | LFSR Granular | 557.86 | 547.95 | **1.78%** |
| 166 | LFSR Rhythmic Harmonics | 874.88 | 878.65 | -0.43% |
| 167 | LFSR Spectral Shift | 203.21 | 204.28 | -0.53% |
| 168 | LFSR Euclidean Beat | 136.91 | 137.52 | -0.45% |
| 169 | LFSR Feedback Synth | 165.44 | 160.94 | **2.72%** |
| 170 | LFSR Algorithmic Lead | 207.24 | 213.18 | -2.87% |
| 171 | LFSR Morphing Pad | 933.43 | 882.79 | **5.43%** |
| 172 | LFSR Breakbeat | 153.89 | 155.08 | -0.77% |
| 173 | LFSR Probability Gate | 156.20 | 154.79 | **0.90%** |
| 174 | LFSR Polyrhythmic Chaos | 185.90 | 186.30 | -0.22% |
| 175 | LFSR Glitch Matrix | 569.43 | 564.53 | **0.86%** |
| 176 | Pac-Man Wakka | 81.08 | 80.49 | **0.73%** |
| 177 | Pac-Man Power Pellet | 88.09 | 88.97 | -1.00% |
| 178 | Pac-Man Death | 79.27 | 78.75 | **0.66%** |
| 179 | Pac-Man Ghost | 84.61 | 82.06 | **3.01%** |
| 180 | Space Invaders Shot | 99.94 | 97.92 | **2.02%** |
| 181 | Space Invaders March | 68.88 | 67.93 | **1.38%** |
| 182 | Space Invaders UFO | 94.86 | 94.16 | **0.74%** |
| 183 | Space Invaders Explosion | 99.39 | 101.00 | -1.62% |
| 184 | Asteroids Thrust | 147.60 | 146.13 | **1.00%** |
| 185 | Asteroids Shoot | 106.10 | 103.29 | **2.65%** |
| 186 | Asteroids Explosion | 117.79 | 122.67 | -4.14% |
| 187 | Asteroids Hyperspace | 119.52 | 120.14 | -0.52% |
| 188 | Galaxian Attack | 77.93 | 78.61 | -0.87% |
| 189 | Galaxian Formation | 97.57 | 93.17 | **4.51%** |
| 190 | Centipede Laser | 115.95 | 111.00 | **4.27%** |
| 191 | Centipede Flea Drop | 95.22 | 88.62 | **6.93%** |
| 192 | Defender Thrust | 140.95 | 138.94 | **1.43%** |
| 193 | Defender Smart Bomb | 114.32 | 115.37 | -0.92% |
| 194 | Frogger Hop | 100.27 | 97.68 | **2.58%** |
| 195 | Frogger Traffic | 105.60 | 104.39 | **1.15%** |
| 196 | Donkey Kong Hammer | 108.39 | 101.03 | **6.79%** |
| 197 | Donkey Kong Jump | 96.68 | 89.69 | **7.23%** |
| 198 | Missile Command Explosion | 118.16 | 122.67 | -3.82% |
| 199 | Tempest Shoot | 110.07 | 107.84 | **2.03%** |
| 200 | Tempest Flip | 80.48 | 81.34 | -1.07% |
| 201 | Berzerk Robot Voice | 123.00 | 118.60 | **3.58%** |
| 202 | Robotron Shoot | 115.48 | 108.79 | **5.79%** |
| 203 | Phoenix Bird Cry | 99.17 | 95.38 | **3.82%** |
| 204 | Gorf Laser | 105.84 | 102.24 | **3.40%** |
| 205 | Scramble Engine | 142.30 | 140.88 | **1.00%** |
| 206 | Zaxxon Alarm | 96.02 | 96.62 | -0.62% |
| 207 | Moon Patrol Bounce | 118.83 | 122.04 | -2.70% |
| 208 | POKEY Pure Tone | 78.36 | 76.03 | **2.97%** |
| 209 | POKEY Filtered Noise | 120.49 | 111.70 | **7.30%** |
| 210 | POKEY Distorted Bass | 68.16 | 65.62 | **3.73%** |
| 211 | POKEY Laser Zap | 127.32 | 124.90 | **1.90%** |
| 212 | POKEY Explosion | 110.84 | 108.73 | **1.90%** |
| 213 | POKEY Engine Rumble | 138.82 | 139.44 | -0.45% |
| 214 | POKEY Bit Crush Lead | 79.91 | 82.12 | -2.77% |
| 215 | POKEY Coin Pickup | 90.51 | 90.92 | -0.45% |
| 216 | POKEY Jump Sound | 109.58 | 104.64 | **4.51%** |
| 217 | POKEY Chirp Bird | 133.68 | 128.90 | **3.58%** |
| 218 | POKEY Alien Voice | 149.94 | 147.77 | **1.45%** |
| 219 | POKEY Power Up | 99.92 | 99.05 | **0.87%** |
| 220 | POKEY Hit Sound | 106.99 | 104.03 | **2.77%** |
| 221 | POKEY Sweep Down | 90.01 | 88.01 | **2.22%** |
| 222 | POKEY Poly Counter | 111.27 | 113.83 | -2.30% |
| 223 | POKEY Four Channel | 133.36 | 132.07 | **0.97%** |
| 224 | POKEY 4-bit Noise (64k) | 82.28 | 81.70 | **0.70%** |
| 225 | POKEY 5-bit Noise (64k) | 83.11 | 81.47 | **1.97%** |
| 226 | POKEY 17-bit Noise (64k) | 82.10 | 80.41 | **2.06%** |
| 227 | POKEY 9-bit Noise (15k) | 80.16 | 79.40 | **0.95%** |
| 228 | POKEY Filtered 4-bit (Fast) | 88.05 | 86.76 | **1.47%** |
| 229 | POKEY Filtered 5-bit (Fast) | 88.26 | 86.83 | **1.62%** |
| 230 | POKEY Tone + 4-bit (64k) | 106.93 | 106.96 | -0.03% |
| 231 | POKEY Tone + 5-bit (64k) | 107.79 | 108.79 | -0.93% |
| 232 | POKEY Tone + 17-bit (64k) | 106.21 | 106.85 | -0.60% |
| 233 | POKEY 4(64k)+5(15k) Combined | 161.12 | 164.11 | -1.86% |
| 234 | POKEY "High Pass" 4-bit (Fast) | 92.98 | 91.11 | **2.01%** |
| 235 | POKEY 64kHz Noise (17-bit) | 83.23 | 81.02 | **2.66%** |
| 236 | POKEY 15kHz Noise (9-bit) | 81.43 | 79.32 | **2.59%** |
| 237 | POKEY Engine Sound (Noise Gated) | 166.39 | 166.46 | -0.04% |
| 238 | POKEY Explosion (Decaying Rate/Vol) | 214.86 | 231.07 | -7.54% |
| 239 | POKEY "Multi-Channel" (Mixed) | 244.00 | 241.69 | **0.95%** |
| 240 | Logic: PWM Hash | 60.36 | 61.39 | -1.71% |
| 241 | Sample & Hold Sine | 39.59 | 38.58 | **2.55%** |
| 242 | Digital Saw | 28.54 | 26.87 | **5.85%** |
| 243 | Glitch Step | 25.58 | 25.50 | **0.31%** |
| 244 | Weird: Gap | 25.79 | 25.64 | **0.58%** |
| 245 | Noise: White-ish | 16.79 | 32.70 | -94.76% |
| 246 | Noise: S&H | 156.26 | 155.70 | **0.36%** |
| 247 | Fibonacci Series | 69.96 | 71.06 | -1.57% |
| 248 | Logistic Chaos | 64.01 | 65.34 | -2.08% |
| 249 | Chebyshev 4th | 70.95 | 71.73 | -1.10% |
| 250 | Tanh Fold | 59.87 | 62.04 | -3.62% |
| 251 | Exp FM | 52.32 | 52.73 | -0.78% |
| 252 | Chaotic Map | 59.00 | 60.78 | -3.02% |
| 253 | Pseudo-LPG | 75.47 | 75.41 | **0.08%** |
| 254 | Harmonic Steps | 40.85 | 40.76 | **0.22%** |
| 255 | Vocal Formant 2 | 39.82 | 40.60 | -1.96% |

## Summary

* **Total Patches Benchmarked:** 256
* **Average Time per Sample (v1.9.25):** 132.23 ns
* **Average Time per Sample (v1.9.26):** 134.21 ns
* **Overall Performance Improvement:** **-1.50%**
