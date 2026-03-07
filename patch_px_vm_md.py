with open('doc/px_vm.md', 'r') as f:
    text = f.read()

math_funcs_tail = """- **`rand()`**
  Returns a new pseudo-random floating-point value between 0.0 and 1.0 every time it is executed. Unlike `RAND_OFFSET`, this function is not constant for the duration of a note.
  - **Example**: `(rand() - 0.5) * 0.1` adds a small amount of random noise to the signal.
"""

new_math_funcs = math_funcs_tail + """
**Taming Functions (Branchless)**
These functions are compiled down to hardware-accelerated C math primitives (`fminf`, `fmaxf`, and `fmaf`), making them ideal for high-performance modulation:

- **`clamp(value, min, max)`**: Clamps the `value` strictly within the `[min, max]` range.
  - **Example**: `clamp(prob(0.5, MOD_A, 0.0), 0.2, 0.8)` creates a bounded, probabilistic modulation.
- **`mix(param, v1, v2)`**: Linearly interpolates between two expressions, `v1` and `v2`, driven by a mixing `param` (safely clamped between `0.0` and `1.0`).
  - **Example**: `mix(MOD_A, sin(x), saw(x))` crossfades between a sine wave and a sawtooth wave.
- **`ramp(start, end, time)`**: Linearly interpolates from `start` to `end` driven by a `time` or envelope progress value (clamped between `0.0` and `1.0`).
  - **Example**: `ramp(0.0, 1.0, MOD_B) * sin(x)` applies an amplitude swell to a sine wave.
"""

if "Taming Functions (Branchless)" not in text:
    text = text.replace(math_funcs_tail, new_math_funcs)

with open('doc/px_vm.md', 'w') as f:
    f.write(text)
