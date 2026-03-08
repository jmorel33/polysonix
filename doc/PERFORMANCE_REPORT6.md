# Polysonix VM v1.9.26 Performance Report

This report compares the execution time of the new Iterative Sigma / Stack-Peek VM (v1.9.26) against the v1.9.16 (FMA Peak) baseline.
Measurements are averaged over 10 runs to eliminate OS and cache noise.

| Patch ID | Name | v1.9.16 (Before) (ns) | v1.9.26 (After) (ns) | Improvement |
| :--- | :--- | :--- | :--- | :--- |
| 0 | Triangle Up | 49.74 | 50.30 | -1.12% |
| 1 | Triangle Down | 51.25 | 50.38 | **1.71%** |
| 2 | Sine Up | 95.16 | 89.33 | **6.13%** |
| 3 | Sine Down | 97.31 | 91.88 | **5.59%** |
| 4 | Square Up | 55.58 | 55.66 | -0.13% |
| 5 | Square Down | 54.49 | 55.42 | -1.72% |
| 6 | Saw Rising | 71.56 | 65.79 | **8.06%** |
| 7 | Saw Falling | 71.97 | 72.07 | -0.14% |
| 8 | Saw/Sine Up | 87.27 | 73.98 | **15.23%** |
| 9 | Sine/Saw Down | 92.33 | 77.81 | **15.72%** |
| 10 | Square/Sine Up | 79.21 | 67.35 | **14.97%** |
| 11 | Sine/Square Down | 79.48 | 66.93 | **15.79%** |
| 12 | Saw/Triangle Up | 44.10 | 39.64 | **10.12%** |
| 13 | Triangle/Saw Down | 37.70 | 36.80 | **2.40%** |
| 14 | Triangle/Sine Up | 68.42 | 57.11 | **16.54%** |
| 15 | Sine/Triangle Down | 71.42 | 58.72 | **17.79%** |
| 16 | Clipped Sine | 48.79 | 45.45 | **6.84%** |
| 17 | Rectified Sine | 48.59 | 48.52 | **0.14%** |
| 18 | Sine * Saw | 53.82 | 53.59 | **0.43%** |
| 19 | Overload Spark | 74.64 | 72.09 | **3.41%** |
| 20 | Overfolded Saw | 63.22 | 57.91 | **8.39%** |
| 21 | Clipped Chaos | 73.35 | 70.49 | **3.90%** |
| 22 | Wavefolder Sim (A=Fold B=Bias) | 77.60 | 76.72 | **1.14%** |
| 23 | Triangle Fold | 41.21 | 41.77 | -1.36% |
| 24 | Math: Tanh Drive | 33.71 | 33.75 | -0.12% |
| 25 | Math: Cubic | 32.25 | 32.27 | -0.06% |
| 26 | Math: Rectified | 24.66 | 25.21 | -2.25% |
| 27 | Math: Sinc | 27.82 | 26.75 | **3.85%** |
| 28 | Weird: Step-Slope | 21.64 | 21.88 | -1.11% |
| 29 | Gritty Bass | 54.60 | 53.11 | **2.74%** |
| 30 | Hybrid Saw*Sine | 46.44 | 45.61 | **1.80%** |
| 31 | Razor Pulse | 65.50 | 64.43 | **1.63%** |
| 32 | Pulse 25% | 47.94 | 49.14 | -2.51% |
| 33 | Pulse 75% | 47.66 | 48.77 | -2.34% |
| 34 | Staircase 4 Step | 70.41 | 70.20 | **0.29%** |
| 35 | Bit Crush Bomb | 80.00 | 81.03 | -1.28% |
| 36 | Bit-Crushed Square | 62.22 | 59.14 | **4.95%** |
| 37 | Pulse Train Wreck | 94.51 | 93.53 | **1.04%** |
| 38 | Narrow | 38.94 | 38.17 | **1.98%** |
| 39 | Quantized Saw 8 | 37.41 | 36.11 | **3.49%** |
| 40 | PWM Synth (A=Width B=Sub) | 62.33 | 63.57 | -1.99% |
| 41 | PWM Gate | 41.45 | 37.84 | **8.70%** |
| 42 | Harmonic Switch | 68.51 | 64.34 | **6.09%** |
| 43 | Multi-Gate | 67.61 | 67.40 | **0.31%** |
| 44 | Bitwise Staircase | 29.71 | 28.75 | **3.23%** |
| 45 | Bitwise XOR Wave | 62.54 | 63.53 | -1.58% |
| 46 | Hard Quantize Sine | 38.84 | 38.79 | **0.13%** |
| 47 | Comparator Fuzz | 40.26 | 40.92 | -1.63% |
| 48 | Warp Speed | 33.43 | 31.00 | **7.25%** |
| 49 | Ghost Wail | 46.33 | 46.61 | -0.60% |
| 50 | Laser Malfunction | 60.96 | 55.08 | **9.65%** |
| 51 | Hyperspace Glitch | 58.81 | 59.35 | -0.92% |
| 52 | Shredded Saw | 54.74 | 53.47 | **2.32%** |
| 53 | Glitch Sine | 69.49 | 70.10 | -0.88% |
| 54 | 4-Segment Bump | 54.57 | 54.97 | -0.72% |
| 55 | Bird Call AM | 51.36 | 48.98 | **4.62%** |
| 56 | Phase Distortion (A=Amt B=Shape) | 72.51 | 65.55 | **9.61%** |
| 57 | Chaos Sine (A=ModRate B=ModAmt) | 47.31 | 47.40 | -0.19% |
| 58 | Phase Glitch | 49.23 | 50.25 | -2.06% |
| 59 | Phase Distortion Wave | 167.15 | 167.57 | -0.25% |
| 60 | PD: Resonant | 36.36 | 36.76 | -1.09% |
| 61 | PD: Wrap | 36.66 | 37.23 | -1.55% |
| 62 | PD: Spike | 29.36 | 30.39 | -3.51% |
| 63 | PD: Windowed | 33.95 | 34.91 | -2.81% |
| 64 | Classic FM EP (A=Index B=Detune) | 76.18 | 76.69 | -0.66% |
| 65 | FM Bass Growl (A=Fdbk B=Index) | 79.43 | 78.34 | **1.38%** |
| 66 | Freq Shifter FM (A=Shift B=Index) | 60.23 | 60.95 | -1.19% |
| 67 | Complex FM A=Index B=ModFreq | 56.74 | 52.06 | **8.24%** |
| 68 | FM Pluck | 140.46 | 141.15 | -0.49% |
| 69 | FM Pitched Grit | 104.05 | 103.81 | **0.23%** |
| 70 | FM Dynamic Lead | 171.76 | 171.80 | -0.02% |
| 71 | FM Glassy Evolve | 99.71 | 102.22 | -2.52% |
| 72 | FM: Classic EP (A=Tine B=Bell C=Ratio) | 81.49 | 82.00 | -0.63% |
| 73 | FM: Growl Bass (A=Index B=Fdbk C=Ratio) | 73.55 | 74.20 | -0.89% |
| 74 | FM: Deep Sub | 32.23 | 32.77 | -1.68% |
| 75 | FM: Talker | 41.81 | 42.34 | -1.28% |
| 76 | FM: Feedback Sim | 56.70 | 57.45 | -1.31% |
| 77 | FM: Cascaded | 62.61 | 62.03 | **0.93%** |
| 78 | FM: Vowel-ish | 38.69 | 40.45 | -4.56% |
| 79 | FM: Sci-Fi Drone (A=Evolve B=Chaos C=Pitch) | 67.65 | 67.94 | -0.44% |
| 80 | FM Metallic Bell (A=Decay B=Ratio) | 95.53 | 92.16 | **3.53%** |
| 81 | FM Hollow Drone (A=ModMix B=ModRatio) | 91.75 | 84.33 | **8.09%** |
| 82 | FM Harsh Noise Sweep (A=Sweep B=Intensity) | 103.01 | 102.69 | **0.31%** |
| 83 | FM Soft Pad (A=Brightness B=Chorus) | 115.81 | 116.82 | -0.88% |
| 84 | FM Bipolar Sweep Pad | 83.81 | 83.74 | **0.08%** |
| 85 | FM Clangorous Hit (A=Metal B=Dissonance) | 99.96 | 101.48 | -1.53% |
| 86 | FM: Noise | 59.26 | 58.72 | **0.92%** |
| 87 | FM Evolving SciFi (A=Evolve B=Harmonics) | 95.46 | 96.34 | -0.92% |
| 88 | FM: Metallic Bell (A=Decay B=Index2 C=Ratio) | 101.30 | 101.03 | **0.26%** |
| 89 | FM: Glitchy Noise (A=Index B=Bit C=Rate) | 90.26 | 92.79 | -2.80% |
| 90 | FM: Metallic 1 | 40.35 | 40.59 | -0.61% |
| 91 | FM: Metallic 2 | 40.75 | 40.78 | -0.09% |
| 92 | Weird: AM Chaos | 33.95 | 34.59 | -1.87% |
| 93 | Sci-Fi Drone | 192.61 | 191.52 | **0.57%** |
| 94 | Evolving Metallic Bell | 596.35 | 602.21 | -0.98% |
| 95 | Alien Communication | 249.12 | 250.88 | -0.71% |
| 96 | Sine Harmonics | 73.58 | 74.94 | -1.84% |
| 97 | Harmonic Noise Blast | 82.80 | 83.45 | -0.79% |
| 98 | Brass | 112.56 | 116.72 | -3.69% |
| 99 | Bowed String | 127.25 | 129.50 | -1.77% |
| 100 | Additive Square | 84.91 | 84.00 | **1.07%** |
| 101 | Electric Pianoish | 80.53 | 80.12 | **0.51%** |
| 102 | Classic Pad | 80.15 | 80.15 | **0.01%** |
| 103 | Additive Saw (A=Harms B=Shape) | 81.37 | 82.13 | -0.94% |
| 104 | Random Phase Additive (A=RndAmt B=Harm) | 124.32 | 125.34 | -0.82% |
| 105 | Grit Additive (A=Grit B=Tone) | 138.64 | 136.17 | **1.78%** |
| 106 | Simple Minor Triad | 101.63 | 101.10 | **0.52%** |
| 107 | Add: Spec 1 | 64.31 | 61.80 | **3.90%** |
| 108 | Add: Spec 2 | 64.61 | 62.72 | **2.93%** |
| 109 | Add: Bell | 49.21 | 47.22 | **4.04%** |
| 110 | Add: Organ | 64.14 | 63.06 | **1.68%** |
| 111 | Add: Random Phase | 47.09 | 46.69 | **0.86%** |
| 112 | Formantish | 63.20 | 63.27 | -0.11% |
| 113 | Vocal Ah | 99.55 | 100.53 | -0.98% |
| 114 | Reso Filter Sweep (A=Reso B=Cutoff) | 83.86 | 83.44 | **0.49%** |
| 115 | Formant Vowel (A=Phsr1 B=Phsr2) | 523.76 | 456.40 | **12.86%** |
| 116 | Sync Sweep No Slant | 42.52 | 40.22 | **5.41%** |
| 117 | Sync Sweep Cos Shape | 54.52 | 50.91 | **6.63%** |
| 118 | Smoothed Sync (A=SyncFreq B=Duty) | 68.42 | 68.56 | -0.20% |
| 119 | Limited Sync (A=SyncFreq B=Duty) | 57.26 | 51.89 | **9.37%** |
| 120 | Sync Sweep (A=SyncFreq B=Duty) | 57.31 | 51.89 | **9.45%** |
| 121 | Oooh Choir Formant | 1747.56 | 1788.55 | -2.35% |
| 122 | PD Vocal Formant | 28.32 | 28.91 | -2.08% |
| 123 | Sync Soft | 35.38 | 40.84 | -15.42% |
| 124 | Fractal Sine | 61.56 | 63.11 | -2.51% |
| 125 | FM Breathy Flute (A=Air B=PitchMod) | 93.97 | 104.15 | -10.83% |
| 126 | Add: Saw 8 | 273.96 | 274.56 | -0.22% |
| 127 | Add: Square 8 | 286.29 | 282.49 | **1.33%** |
| 128 | Kick Drum | 61.51 | 62.78 | -2.07% |
| 129 | Snare Drum | 72.01 | 70.66 | **1.88%** |
| 130 | Clap | 88.62 | 87.82 | **0.90%** |
| 131 | Tom Drum | 61.65 | 62.78 | -1.84% |
| 132 | Cymbalish | 67.17 | 68.77 | -2.38% |
| 133 | Double Waves | 96.29 | 108.82 | -13.01% |
| 134 | Metal Impact | 69.11 | 70.60 | -2.16% |
| 135 | Bell Tone | 76.64 | 78.38 | -2.27% |
| 136 | Metallic Perc | 67.14 | 68.46 | -1.97% |
| 137 | Sigma Bell (A=Decay B=Metal) | 276.36 | 279.86 | -1.26% |
| 138 | Classic Noise Sim | 136.41 | 129.99 | **4.71%** |
| 139 | Distorted Pitch | 119.95 | 118.35 | **1.33%** |
| 140 | Gritty Rumble Noise | 110.84 | 146.14 | -31.85% |
| 141 | Filtered Static Noise | 437.78 | 531.04 | -21.30% |
| 142 | Wooden Percussion | 77.07 | 79.27 | -2.85% |
| 143 | Glitchy Percussion | 297.64 | 299.26 | -0.55% |
| 144 | Plucked String (A=Damp B=Body) | 199.99 | 196.75 | **1.62%** |
| 145 | Sigma A=End B=Decay | 86.38 | 87.91 | -1.78% |
| 146 | Noisy Pad (A=NoiseAmt B=Flt) | 203.79 | 217.43 | -6.69% |
| 147 | Rich String Ensemble | 986.10 | 985.03 | **0.11%** |
| 148 | Mellow Brass Section | 529.01 | 542.53 | -2.56% |
| 149 | Jittery Inharmonic Pitch | 668.93 | 636.83 | **4.80%** |
| 150 | LFSR Granular Texture | 238.71 | 230.92 | **3.27%** |
| 151 | Morphing Harmonics | 1757.91 | 1855.81 | -5.57% |
| 152 | Breathing Pad | 139.19 | 136.35 | **2.04%** |
| 153 | Chaotic Oscillator | 169.52 | 165.85 | **2.16%** |
| 154 | Crystalline Arpeggio | 1061.17 | 1116.11 | -5.18% |
| 155 | Add: Shepard Cycle | 42.92 | 42.62 | **0.71%** |
| 156 | Water Droplet | 69.29 | 69.19 | **0.15%** |
| 157 | Alien Chatter | 82.63 | 83.72 | -1.32% |
| 158 | Weird: Chirp | 23.50 | 23.71 | -0.91% |
| 159 | Wind AM | 49.47 | 50.11 | -1.29% |
| 160 | LFSR Rhythm Gate | 109.23 | 105.00 | **3.87%** |
| 161 | LFSR Harmonic Chaos | 802.99 | 800.04 | **0.37%** |
| 162 | LFSR Digital Texture | 204.75 | 213.69 | -4.37% |
| 163 | LFSR Poly Rhythm | 193.92 | 214.19 | -10.46% |
| 164 | LFSR Phase Modulation | 142.71 | 132.32 | **7.28%** |
| 165 | LFSR Granular | 557.86 | 579.89 | -3.95% |
| 166 | LFSR Rhythmic Harmonics | 874.88 | 931.50 | -6.47% |
| 167 | LFSR Spectral Shift | 203.21 | 199.91 | **1.62%** |
| 168 | LFSR Euclidean Beat | 136.91 | 149.17 | -8.95% |
| 169 | LFSR Feedback Synth | 165.44 | 151.94 | **8.16%** |
| 170 | LFSR Algorithmic Lead | 207.24 | 219.47 | -5.90% |
| 171 | LFSR Morphing Pad | 933.43 | 876.02 | **6.15%** |
| 172 | LFSR Breakbeat | 153.89 | 168.23 | -9.32% |
| 173 | LFSR Probability Gate | 156.20 | 154.12 | **1.33%** |
| 174 | LFSR Polyrhythmic Chaos | 185.90 | 211.17 | -13.59% |
| 175 | LFSR Glitch Matrix | 569.43 | 585.38 | -2.80% |
| 176 | Pac-Man Wakka | 81.08 | 80.75 | **0.41%** |
| 177 | Pac-Man Power Pellet | 88.09 | 87.08 | **1.15%** |
| 178 | Pac-Man Death | 79.27 | 80.19 | -1.15% |
| 179 | Pac-Man Ghost | 84.61 | 82.06 | **3.01%** |
| 180 | Space Invaders Shot | 99.94 | 98.96 | **0.98%** |
| 181 | Space Invaders March | 68.88 | 68.86 | **0.03%** |
| 182 | Space Invaders UFO | 94.86 | 95.44 | -0.61% |
| 183 | Space Invaders Explosion | 99.39 | 99.09 | **0.30%** |
| 184 | Asteroids Thrust | 147.60 | 144.03 | **2.42%** |
| 185 | Asteroids Shoot | 106.10 | 105.36 | **0.70%** |
| 186 | Asteroids Explosion | 117.79 | 116.50 | **1.09%** |
| 187 | Asteroids Hyperspace | 119.52 | 117.92 | **1.34%** |
| 188 | Galaxian Attack | 77.93 | 78.06 | -0.17% |
| 189 | Galaxian Formation | 97.57 | 93.32 | **4.35%** |
| 190 | Centipede Laser | 115.95 | 112.56 | **2.92%** |
| 191 | Centipede Flea Drop | 95.22 | 88.02 | **7.56%** |
| 192 | Defender Thrust | 140.95 | 137.94 | **2.14%** |
| 193 | Defender Smart Bomb | 114.32 | 114.71 | -0.34% |
| 194 | Frogger Hop | 100.27 | 97.72 | **2.55%** |
| 195 | Frogger Traffic | 105.60 | 104.36 | **1.17%** |
| 196 | Donkey Kong Hammer | 108.39 | 101.00 | **6.82%** |
| 197 | Donkey Kong Jump | 96.68 | 88.40 | **8.57%** |
| 198 | Missile Command Explosion | 118.16 | 117.19 | **0.83%** |
| 199 | Tempest Shoot | 110.07 | 108.55 | **1.38%** |
| 200 | Tempest Flip | 80.48 | 80.53 | -0.06% |
| 201 | Berzerk Robot Voice | 123.00 | 120.45 | **2.07%** |
| 202 | Robotron Shoot | 115.48 | 110.69 | **4.15%** |
| 203 | Phoenix Bird Cry | 99.17 | 94.68 | **4.53%** |
| 204 | Gorf Laser | 105.84 | 102.90 | **2.78%** |
| 205 | Scramble Engine | 142.30 | 139.86 | **1.72%** |
| 206 | Zaxxon Alarm | 96.02 | 96.56 | -0.56% |
| 207 | Moon Patrol Bounce | 118.83 | 120.04 | -1.02% |
| 208 | POKEY Pure Tone | 78.36 | 79.12 | -0.96% |
| 209 | POKEY Filtered Noise | 120.49 | 111.52 | **7.44%** |
| 210 | POKEY Distorted Bass | 68.16 | 62.61 | **8.14%** |
| 211 | POKEY Laser Zap | 127.32 | 123.28 | **3.18%** |
| 212 | POKEY Explosion | 110.84 | 108.13 | **2.44%** |
| 213 | POKEY Engine Rumble | 138.82 | 135.91 | **2.10%** |
| 214 | POKEY Bit Crush Lead | 79.91 | 79.41 | **0.62%** |
| 215 | POKEY Coin Pickup | 90.51 | 91.02 | -0.56% |
| 216 | POKEY Jump Sound | 109.58 | 103.80 | **5.27%** |
| 217 | POKEY Chirp Bird | 133.68 | 127.10 | **4.92%** |
| 218 | POKEY Alien Voice | 149.94 | 145.71 | **2.82%** |
| 219 | POKEY Power Up | 99.92 | 98.21 | **1.71%** |
| 220 | POKEY Hit Sound | 106.99 | 105.25 | **1.63%** |
| 221 | POKEY Sweep Down | 90.01 | 85.88 | **4.59%** |
| 222 | POKEY Poly Counter | 111.27 | 111.32 | -0.05% |
| 223 | POKEY Four Channel | 133.36 | 133.91 | -0.42% |
| 224 | POKEY 4-bit Noise (64k) | 82.28 | 79.90 | **2.89%** |
| 225 | POKEY 5-bit Noise (64k) | 83.11 | 81.25 | **2.23%** |
| 226 | POKEY 17-bit Noise (64k) | 82.10 | 80.37 | **2.11%** |
| 227 | POKEY 9-bit Noise (15k) | 80.16 | 78.87 | **1.62%** |
| 228 | POKEY Filtered 4-bit (Fast) | 88.05 | 85.95 | **2.38%** |
| 229 | POKEY Filtered 5-bit (Fast) | 88.26 | 86.23 | **2.30%** |
| 230 | POKEY Tone + 4-bit (64k) | 106.93 | 105.60 | **1.24%** |
| 231 | POKEY Tone + 5-bit (64k) | 107.79 | 105.94 | **1.71%** |
| 232 | POKEY Tone + 17-bit (64k) | 106.21 | 104.72 | **1.40%** |
| 233 | POKEY 4(64k)+5(15k) Combined | 161.12 | 166.04 | -3.06% |
| 234 | POKEY "High Pass" 4-bit (Fast) | 92.98 | 90.40 | **2.78%** |
| 235 | POKEY 64kHz Noise (17-bit) | 83.23 | 81.63 | **1.92%** |
| 236 | POKEY 15kHz Noise (9-bit) | 81.43 | 80.00 | **1.75%** |
| 237 | POKEY Engine Sound (Noise Gated) | 166.39 | 164.50 | **1.14%** |
| 238 | POKEY Explosion (Decaying Rate/Vol) | 214.86 | 212.38 | **1.16%** |
| 239 | POKEY "Multi-Channel" (Mixed) | 244.00 | 238.56 | **2.23%** |
| 240 | Logic: PWM Hash | 60.36 | 34.80 | **42.34%** |
| 241 | Sample & Hold Sine | 39.59 | 38.88 | **1.78%** |
| 242 | Digital Saw | 28.54 | 27.09 | **5.10%** |
| 243 | Glitch Step | 25.58 | 27.86 | -8.89% |
| 244 | Weird: Gap | 25.79 | 25.69 | **0.41%** |
| 245 | Noise: White-ish | 16.79 | 32.36 | -92.70% |
| 246 | Noise: S&H | 156.26 | 154.29 | **1.26%** |
| 247 | Fibonacci Series | 69.96 | 70.38 | -0.61% |
| 248 | Logistic Chaos | 64.01 | 65.02 | -1.58% |
| 249 | Chebyshev 4th | 70.95 | 70.78 | **0.24%** |
| 250 | Tanh Fold | 59.87 | 60.87 | -1.67% |
| 251 | Exp FM | 52.32 | 52.42 | -0.19% |
| 252 | Chaotic Map | 59.00 | 60.88 | -3.19% |
| 253 | Pseudo-LPG | 75.47 | 75.80 | -0.43% |
| 254 | Harmonic Steps | 40.85 | 40.98 | -0.33% |
| 255 | Vocal Formant 2 | 39.82 | 40.86 | -2.61% |

## Summary

* **Total Patches Benchmarked:** 256
* **Averaged Over:** 10 test runs
* **Average Time per Sample (v1.9.16):** 132.23 ns
* **Average Time per Sample (v1.9.26):** 132.48 ns
* **Overall Performance Improvement:** **-0.19%**
