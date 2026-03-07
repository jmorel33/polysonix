with open('doc/px_vm.md', 'r') as f:
    text = f.read()

old_toc = """    - [3.5.4. Dynamic Selection Function (select)](#354-dynamic-selection-function-select)
    - [3.5.5. Smooth Interpolated Selection (smooth_select)](#355-smooth-interpolated-selection-smooth_select)
    - [3.5.6. LFSR (Linear-Feedback Shift Register) Functions](#356-lfsr-linear-feedback-shift-register-functions)
    - [3.5.7. Summation Function (sigma)](#357-summation-function-sigma)"""

new_toc = """    - [3.5.4. Dynamic Selection Function (select)](#354-dynamic-selection-function-select)
    - [3.5.5. Taming Functions (clamp, mix, ramp)](#355-taming-functions-clamp-mix-ramp)
    - [3.5.6. Smooth Interpolated Selection (smooth_select)](#356-smooth-interpolated-selection-smooth_select)
    - [3.5.7. LFSR (Linear-Feedback Shift Register) Functions](#357-lfsr-linear-feedback-shift-register-functions)
    - [3.5.8. Summation Function (sigma)](#358-summation-function-sigma)"""

text = text.replace(old_toc, new_toc)

with open('doc/px_vm.md', 'w') as f:
    f.write(text)
