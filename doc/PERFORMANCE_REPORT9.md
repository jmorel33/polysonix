# Polysonix VM Performance Report 9

This report compares the execution time of the VM (v1.9.35) with stack isolation and inverse constant support against the PERFORMANCE_REPORT7.md baseline.
Measurements are averaged over 10 runs to eliminate OS and cache noise.

| Patch ID | Name | Report 7 (Before) (ns) | Current (After) (ns) | Improvement |
| :--- | :--- | :--- | :--- | :--- |
| 0 | Triangle Up | 49.12 | 53.27 | -8.46% |
| 1 | Triangle Down | 49.92 | 52.34 | -4.84% |
| 2 | Sine Up | 94.72 | 89.28 | **5.75%** |
| 3 | Sine Down | 97.88 | 92.90 | **5.09%** |
| 4 | Square Up | 53.96 | 57.36 | -6.30% |
| 5 | Square Down | 54.61 | 58.45 | -7.04% |
| 6 | Saw Rising | 72.11 | 76.29 | -5.80% |
| 7 | Saw Falling | 71.58 | 84.42 | -17.94% |
| 8 | Saw/Sine Up | 86.44 | 121.40 | -40.44% |
| 9 | Sine/Saw Down | 91.57 | 135.90 | -48.41% |
| 10 | Square/Sine Up | 77.69 | 116.29 | -49.68% |
| 11 | Sine/Square Down | 78.38 | 110.29 | -40.71% |
| 12 | Saw/Triangle Up | 39.80 | 69.70 | -75.14% |
| 13 | Triangle/Saw Down | 36.87 | 51.40 | -39.40% |
| 14 | Triangle/Sine Up | 66.72 | 54.06 | **18.97%** |
| 15 | Sine/Triangle Down | 67.01 | 58.67 | **12.45%** |
| 16 | Clipped Sine | 48.74 | 46.61 | **4.37%** |
| 17 | Rectified Sine | 48.05 | 48.82 | -1.60% |
| 18 | Sine * Saw | 54.51 | 60.23 | -10.50% |
| 19 | Overload Spark | 71.13 | 136.21 | -91.49% |
| 20 | Overfolded Saw | 62.69 | 58.06 | **7.39%** |
| 21 | Clipped Chaos | 71.37 | 100.29 | -40.53% |
| 22 | Wavefolder Sim (A=Fold B=Bias) | 76.28 | 82.81 | -8.56% |
| 23 | Triangle Fold | 41.48 | 41.72 | -0.57% |
| 24 | Math: Tanh Drive | 33.53 | 35.47 | -5.80% |
| 25 | Math: Cubic | 32.11 | 31.21 | **2.80%** |
| 26 | Math: Rectified | 24.79 | 25.49 | -2.82% |
| 27 | Math: Sinc | 25.90 | 29.56 | -14.15% |
| 28 | Weird: Step-Slope | 21.59 | 31.30 | -44.97% |
| 29 | Gritty Bass | 53.94 | 55.79 | -3.43% |
| 30 | Hybrid Saw*Sine | 45.63 | 46.70 | -2.34% |
| 31 | Razor Pulse | 64.54 | 83.57 | -29.49% |
| 32 | Pulse 25% | 47.60 | 47.96 | -0.76% |
| 33 | Pulse 75% | 47.16 | 47.85 | -1.47% |
| 34 | Staircase 4 Step | 69.41 | 128.84 | -85.62% |
| 35 | Bit Crush Bomb | 79.56 | 105.57 | -32.69% |
| 36 | Bit-Crushed Square | 62.13 | 60.64 | **2.40%** |
| 37 | Pulse Train Wreck | 95.90 | 140.20 | -46.19% |
| 38 | Narrow | 38.11 | 58.77 | -54.20% |
| 39 | Quantized Saw 8 | 36.20 | 39.58 | -9.34% |
| 40 | PWM Synth (A=Width B=Sub) | 62.37 | 63.64 | -2.04% |
| 41 | PWM Gate | 38.05 | 51.03 | -34.13% |
| 42 | Harmonic Switch | 64.36 | 65.09 | -1.13% |
| 43 | Multi-Gate | 67.60 | 67.21 | **0.58%** |
| 44 | Bitwise Staircase | 28.33 | 28.92 | -2.08% |
| 45 | Bitwise XOR Wave | 62.45 | 63.47 | -1.63% |
| 46 | Hard Quantize Sine | 37.97 | 38.55 | -1.54% |
| 47 | Comparator Fuzz | 40.70 | 44.80 | -10.07% |
| 48 | Warp Speed | 30.65 | 31.12 | -1.55% |
| 49 | Ghost Wail | 46.78 | 46.64 | **0.30%** |
| 50 | Laser Malfunction | 58.98 | 55.25 | **6.32%** |
| 51 | Hyperspace Glitch | 58.55 | 59.02 | -0.81% |
| 52 | Shredded Saw | 54.58 | 53.20 | **2.52%** |
| 53 | Glitch Sine | 67.78 | 69.45 | -2.46% |
| 54 | 4-Segment Bump | 54.97 | 55.21 | -0.44% |
| 55 | Bird Call AM | 48.34 | 48.73 | -0.82% |
| 56 | Phase Distortion (A=Amt B=Shape) | 65.77 | 65.58 | **0.30%** |
| 57 | Chaos Sine (A=ModRate B=ModAmt) | 47.11 | 47.38 | -0.58% |
| 58 | Phase Glitch | 49.83 | 50.29 | -0.92% |
| 59 | Phase Distortion Wave | 168.00 | 167.82 | **0.11%** |
| 60 | PD: Resonant | 36.38 | 36.79 | -1.13% |
| 61 | PD: Wrap | 37.02 | 37.13 | -0.30% |
| 62 | PD: Spike | 30.05 | 30.06 | -0.03% |
| 63 | PD: Windowed | 34.31 | 34.81 | -1.47% |
| 64 | Classic FM EP (A=Index B=Detune) | 76.37 | 76.94 | -0.75% |
| 65 | FM Bass Growl (A=Fdbk B=Index) | 77.94 | 79.39 | -1.85% |
| 66 | Freq Shifter FM (A=Shift B=Index) | 59.98 | 61.06 | -1.80% |
| 67 | Complex FM A=Index B=ModFreq | 51.69 | 52.75 | -2.05% |
| 68 | FM Pluck | 141.39 | 140.81 | **0.41%** |
| 69 | FM Pitched Grit | 104.63 | 103.80 | **0.80%** |
| 70 | FM Dynamic Lead | 172.84 | 171.38 | **0.84%** |
| 71 | FM Glassy Evolve | 101.64 | 102.12 | -0.47% |
| 72 | FM: Classic EP (A=Tine B=Bell C=Ratio) | 81.69 | 82.12 | -0.53% |
| 73 | FM: Growl Bass (A=Index B=Fdbk C=Ratio) | 73.81 | 75.22 | -1.91% |
| 74 | FM: Deep Sub | 32.37 | 32.55 | -0.56% |
| 75 | FM: Talker | 41.25 | 41.78 | -1.28% |
| 76 | FM: Feedback Sim | 57.43 | 57.34 | **0.17%** |
| 77 | FM: Cascaded | 62.61 | 61.89 | **1.16%** |
| 78 | FM: Vowel-ish | 39.92 | 40.66 | -1.84% |
| 79 | FM: Sci-Fi Drone (A=Evolve B=Chaos C=Pitch) | 68.15 | 68.32 | -0.25% |
| 80 | FM Metallic Bell (A=Decay B=Ratio) | 92.33 | 95.16 | -3.06% |
| 81 | FM Hollow Drone (A=ModMix B=ModRatio) | 88.13 | 84.23 | **4.42%** |
| 82 | FM Harsh Noise Sweep (A=Sweep B=Intensity) | 101.23 | 103.46 | -2.20% |
| 83 | FM Soft Pad (A=Brightness B=Chorus) | 116.03 | 117.94 | -1.65% |
| 84 | FM Bipolar Sweep Pad | 83.62 | 84.04 | -0.50% |
| 85 | FM Clangorous Hit (A=Metal B=Dissonance) | 100.47 | 101.20 | -0.73% |
| 86 | FM: Noise | 58.86 | 58.39 | **0.81%** |
| 87 | FM Evolving SciFi (A=Evolve B=Harmonics) | 96.12 | 96.50 | -0.39% |
| 88 | FM: Metallic Bell (A=Decay B=Index2 C=Ratio) | 103.87 | 102.94 | **0.89%** |
| 89 | FM: Glitchy Noise (A=Index B=Bit C=Rate) | 92.35 | 92.91 | -0.61% |
| 90 | FM: Metallic 1 | 40.39 | 40.87 | -1.19% |
| 91 | FM: Metallic 2 | 40.75 | 40.94 | -0.47% |
| 92 | Weird: AM Chaos | 34.33 | 34.68 | -1.02% |
| 93 | Sci-Fi Drone | 192.88 | 193.32 | -0.23% |
| 94 | Evolving Metallic Bell | 595.18 | 605.55 | -1.74% |
| 95 | Alien Communication | 245.44 | 251.99 | -2.67% |
| 96 | Sine Harmonics | 74.20 | 74.80 | -0.81% |
| 97 | Harmonic Noise Blast | 82.23 | 82.77 | -0.65% |
| 98 | Brass | 115.87 | 115.55 | **0.28%** |
| 99 | Bowed String | 122.79 | 126.92 | -3.36% |
| 100 | Additive Square | 83.28 | 84.76 | -1.78% |
| 101 | Electric Pianoish | 79.00 | 80.31 | -1.66% |
| 102 | Classic Pad | 78.64 | 79.66 | -1.29% |
| 103 | Additive Saw (A=Harms B=Shape) | 82.17 | 81.62 | **0.67%** |
| 104 | Random Phase Additive (A=RndAmt B=Harm) | 122.29 | 127.25 | -4.06% |
| 105 | Grit Additive (A=Grit B=Tone) | 133.88 | 136.41 | -1.89% |
| 106 | Simple Minor Triad | 100.12 | 101.06 | -0.94% |
| 107 | Add: Spec 1 | 61.47 | 61.53 | -0.09% |
| 108 | Add: Spec 2 | 62.02 | 62.57 | -0.89% |
| 109 | Add: Bell | 46.95 | 47.38 | -0.92% |
| 110 | Add: Organ | 62.18 | 62.95 | -1.25% |
| 111 | Add: Random Phase | 46.40 | 46.55 | -0.32% |
| 112 | Formantish | 62.08 | 62.54 | -0.74% |
| 113 | Vocal Ah | 99.65 | 100.31 | -0.66% |
| 114 | Reso Filter Sweep (A=Reso B=Cutoff) | 83.95 | 83.29 | **0.79%** |
| 115 | Formant Vowel | 452.50 | 85.68 | **81.07%** |
| 116 | Sync Sweep No Slant | 43.58 | 40.56 | **6.93%** |
| 117 | Sync Sweep Cos Shape | 53.88 | 50.73 | **5.86%** |
| 118 | Smoothed Sync (A=SyncFreq B=Duty) | 67.56 | 68.06 | -0.74% |
| 119 | Limited Sync (A=SyncFreq B=Duty) | 55.92 | 52.31 | **6.45%** |
| 120 | Sync Sweep (A=SyncFreq B=Duty) | 55.80 | 52.45 | **5.99%** |
| 121 | Oooh Choir Formant | 1766.28 | 1759.42 | **0.39%** |
| 122 | PD Vocal Formant | 28.38 | 28.73 | -1.22% |
| 123 | Sync Soft | 35.42 | 40.71 | -14.94% |
| 124 | Fractal Sine | 62.98 | 62.74 | **0.38%** |
| 125 | FM Breathy Flute (A=Air B=PitchMod) | 102.80 | 103.05 | -0.24% |
| 126 | Add: Saw 8 | 268.90 | 273.12 | -1.57% |
| 127 | Add: Square 8 | 280.13 | 282.77 | -0.94% |
| 128 | Kick Drum | 61.94 | 62.38 | -0.71% |
| 129 | Snare Drum | 71.36 | 70.25 | **1.56%** |
| 130 | Clap | 88.56 | 87.62 | **1.06%** |
| 131 | Tom Drum | 62.04 | 62.21 | -0.27% |
| 132 | Cymbalish | 67.64 | 68.03 | -0.58% |
| 133 | Double Waves | 97.72 | 108.81 | -11.34% |
| 134 | Metal Impact | 69.56 | 69.93 | -0.53% |
| 135 | Bell Tone | 77.16 | 77.62 | -0.60% |
| 136 | Metallic Perc | 67.55 | 67.93 | -0.56% |
| 137 | Sigma Bell (A=Decay B=Metal) | 274.71 | 279.83 | -1.86% |
| 138 | Classic Noise Sim | 132.46 | 129.14 | **2.51%** |
| 139 | Distorted Pitch | 117.86 | 118.04 | -0.15% |
| 140 | Gritty Rumble Noise | 144.86 | 144.22 | **0.44%** |
| 141 | Filtered Static Noise | 532.75 | 526.83 | **1.11%** |
| 142 | Wooden Percussion | 78.16 | 78.38 | -0.29% |
| 143 | Glitchy Percussion | 289.13 | 297.86 | -3.02% |
| 144 | Plucked String (A=Damp B=Body) | 194.98 | 194.77 | **0.11%** |
| 145 | Sigma A=End B=Decay | 86.51 | 86.85 | -0.40% |
| 146 | Noisy Pad (A=NoiseAmt B=Flt) | 219.49 | 214.95 | **2.07%** |
| 147 | Rich String Ensemble | 961.76 | 970.34 | -0.89% |
| 148 | Mellow Brass Section | 533.79 | 542.14 | -1.57% |
| 149 | Jittery Inharmonic Pitch | 618.60 | 624.04 | -0.88% |
| 150 | LFSR Granular Texture | 230.63 | 228.66 | **0.86%** |
| 151 | Morphing Harmonics | 1833.00 | 1869.64 | -2.00% |
| 152 | Breathing Pad | 136.78 | 136.58 | **0.14%** |
| 153 | Chaotic Oscillator | 167.19 | 165.05 | **1.28%** |
| 154 | Crystalline Arpeggio | 1044.03 | 1129.40 | -8.18% |
| 155 | Add: Shepard Cycle | 42.52 | 42.48 | **0.09%** |
| 156 | Water Droplet | 68.79 | 69.03 | -0.35% |
| 157 | Alien Chatter | 82.03 | 82.83 | -0.98% |
| 158 | Weird: Chirp | 23.43 | 23.70 | -1.13% |
| 159 | Wind AM | 49.25 | 49.82 | -1.16% |
| 160 | LFSR Rhythm Gate | 101.86 | 103.43 | -1.54% |
| 161 | LFSR Harmonic Chaos | 799.12 | 818.71 | -2.45% |
| 162 | LFSR Digital Texture | 205.40 | 216.11 | -5.21% |
| 163 | LFSR Poly Rhythm | 192.91 | 215.80 | -11.87% |
| 164 | LFSR Phase Modulation | 134.94 | 133.78 | **0.86%** |
| 165 | LFSR Granular | 546.04 | 589.41 | -7.94% |
| 166 | LFSR Rhythmic Harmonics | 834.32 | 925.80 | -10.96% |
| 167 | LFSR Spectral Shift | 201.70 | 200.58 | **0.56%** |
| 168 | LFSR Euclidean Beat | 137.03 | 148.22 | -8.17% |
| 169 | LFSR Feedback Synth | 160.76 | 152.27 | **5.28%** |
| 170 | LFSR Algorithmic Lead | 209.75 | 217.48 | -3.69% |
| 171 | LFSR Morphing Pad | 882.96 | 883.44 | -0.05% |
| 172 | LFSR Breakbeat | 153.93 | 168.99 | -9.78% |
| 173 | LFSR Probability Gate | 154.04 | 154.78 | -0.48% |
| 174 | LFSR Polyrhythmic Chaos | 182.84 | 211.46 | -15.65% |
| 175 | LFSR Glitch Matrix | 567.89 | 587.02 | -3.37% |
| 176 | Pac-Man Wakka | 80.16 | 80.88 | -0.89% |
| 177 | Pac-Man Power Pellet | 86.70 | 87.92 | -1.40% |
| 178 | Pac-Man Death | 78.75 | 80.61 | -2.37% |
| 179 | Pac-Man Ghost | 81.33 | 83.31 | -2.43% |
| 180 | Space Invaders Shot | 97.75 | 100.85 | -3.17% |
| 181 | Space Invaders March | 67.56 | 69.34 | -2.63% |
| 182 | Space Invaders UFO | 92.67 | 95.09 | -2.61% |
| 183 | Space Invaders Explosion | 98.00 | 100.79 | -2.85% |
| 184 | Asteroids Thrust | 142.93 | 145.55 | -1.83% |
| 185 | Asteroids Shoot | 102.49 | 105.06 | -2.51% |
| 186 | Asteroids Explosion | 115.31 | 118.97 | -3.18% |
| 187 | Asteroids Hyperspace | 116.70 | 119.69 | -2.56% |
| 188 | Galaxian Attack | 77.61 | 77.90 | -0.37% |
| 189 | Galaxian Formation | 92.37 | 93.77 | -1.51% |
| 190 | Centipede Laser | 110.64 | 112.81 | -1.96% |
| 191 | Centipede Flea Drop | 86.88 | 89.23 | -2.70% |
| 192 | Defender Thrust | 137.75 | 138.95 | -0.87% |
| 193 | Defender Smart Bomb | 112.88 | 114.34 | -1.29% |
| 194 | Frogger Hop | 98.19 | 98.93 | -0.75% |
| 195 | Frogger Traffic | 104.59 | 105.02 | -0.41% |
| 196 | Donkey Kong Hammer | 100.32 | 102.86 | -2.54% |
| 197 | Donkey Kong Jump | 87.50 | 88.38 | -1.00% |
| 198 | Missile Command Explosion | 115.88 | 119.15 | -2.82% |
| 199 | Tempest Shoot | 107.15 | 109.85 | -2.52% |
| 200 | Tempest Flip | 81.10 | 80.69 | **0.51%** |
| 201 | Berzerk Robot Voice | 118.87 | 120.28 | -1.19% |
| 202 | Robotron Shoot | 109.78 | 113.12 | -3.05% |
| 203 | Phoenix Bird Cry | 94.06 | 94.60 | -0.58% |
| 204 | Gorf Laser | 101.27 | 103.91 | -2.60% |
| 205 | Scramble Engine | 139.50 | 140.41 | -0.66% |
| 206 | Zaxxon Alarm | 96.18 | 97.19 | -1.06% |
| 207 | Moon Patrol Bounce | 119.07 | 120.62 | -1.31% |
| 208 | POKEY Pure Tone | 76.32 | 78.67 | -3.08% |
| 209 | POKEY Filtered Noise | 110.02 | 111.46 | -1.31% |
| 210 | POKEY Distorted Bass | 61.52 | 62.42 | -1.46% |
| 211 | POKEY Laser Zap | 123.31 | 125.31 | -1.62% |
| 212 | POKEY Explosion | 106.82 | 108.94 | -1.98% |
| 213 | POKEY Engine Rumble | 135.21 | 138.15 | -2.17% |
| 214 | POKEY Bit Crush Lead | 78.99 | 79.35 | -0.46% |
| 215 | POKEY Coin Pickup | 91.00 | 91.04 | -0.04% |
| 216 | POKEY Jump Sound | 103.15 | 104.51 | -1.32% |
| 217 | POKEY Chirp Bird | 127.55 | 128.38 | -0.65% |
| 218 | POKEY Alien Voice | 145.42 | 146.92 | -1.03% |
| 219 | POKEY Power Up | 98.56 | 98.94 | -0.39% |
| 220 | POKEY Hit Sound | 103.25 | 105.08 | -1.77% |
| 221 | POKEY Sweep Down | 85.94 | 86.13 | -0.23% |
| 222 | POKEY Poly Counter | 109.58 | 110.49 | -0.83% |
| 223 | POKEY Four Channel | 133.02 | 134.47 | -1.09% |
| 224 | POKEY 4-bit Noise (64k) | 81.34 | 80.55 | **0.98%** |
| 225 | POKEY 5-bit Noise (64k) | 80.40 | 82.28 | -2.33% |
| 226 | POKEY 17-bit Noise (64k) | 79.52 | 81.41 | -2.37% |
| 227 | POKEY 9-bit Noise (15k) | 77.94 | 78.91 | -1.24% |
| 228 | POKEY Filtered 4-bit (Fast) | 87.14 | 86.27 | **1.00%** |
| 229 | POKEY Filtered 5-bit (Fast) | 87.69 | 87.79 | -0.11% |
| 230 | POKEY Tone + 4-bit (64k) | 104.34 | 105.84 | -1.44% |
| 231 | POKEY Tone + 5-bit (64k) | 105.16 | 106.23 | -1.02% |
| 232 | POKEY Tone + 17-bit (64k) | 104.06 | 104.78 | -0.70% |
| 233 | POKEY 4(64k)+5(15k) Combined | 164.73 | 165.94 | -0.74% |
| 234 | POKEY "High Pass" 4-bit (Fast) | 90.87 | 90.20 | **0.73%** |
| 235 | POKEY 64kHz Noise (17-bit) | 80.78 | 81.88 | -1.37% |
| 236 | POKEY 15kHz Noise (9-bit) | 79.18 | 80.11 | -1.18% |
| 237 | POKEY Engine Sound (Noise Gated) | 165.12 | 164.77 | **0.21%** |
| 238 | POKEY Explosion (Decaying Rate/Vol) | 209.34 | 215.96 | -3.16% |
| 239 | POKEY "Multi-Channel" (Mixed) | 242.36 | 246.54 | -1.72% |
| 240 | Logic: PWM Hash | 61.11 | 35.12 | **42.53%** |
| 241 | Sample & Hold Sine | 38.73 | 38.96 | -0.59% |
| 242 | Digital Saw | 26.78 | 26.98 | -0.73% |
| 243 | Glitch Step | 25.65 | 27.77 | -8.27% |
| 244 | Weird: Gap | 25.73 | 25.61 | **0.47%** |
| 245 | Noise: White-ish | 32.45 | 32.47 | -0.06% |
| 246 | Noise: S&H | 153.74 | 154.74 | -0.65% |
| 247 | Fibonacci Series | 69.95 | 70.24 | -0.41% |
| 248 | Logistic Chaos | 64.83 | 65.03 | -0.31% |
| 249 | Chebyshev 4th | 70.80 | 71.03 | -0.32% |
| 250 | Tanh Fold | 61.23 | 60.64 | **0.96%** |
| 251 | Exp FM | 52.30 | 53.00 | -1.34% |
| 252 | Chaotic Map | 60.65 | 60.90 | -0.40% |
| 253 | Pseudo-LPG | 75.70 | 75.60 | **0.13%** |
| 254 | Harmonic Steps | 40.37 | 40.88 | -1.25% |
| 255 | Vocal Formant 2 | 40.42 | 40.91 | -1.20% |

## Summary

* **Total Patches Benchmarked:** 256
* **Averaged Over:** 10 test runs
* **Average Time per Sample (Report 7):** 130.88 ns
* **Average Time per Sample (Current):** 133.49 ns
* **Overall Performance Improvement:** **-2.00%**
