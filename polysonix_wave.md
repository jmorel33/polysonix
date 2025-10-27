# Polysonix Waveform Scripting Language
   (c) 2025 Jacques Morel
   This software is licensed under the MIT License.

The Polysonix Waveform Scripting Language is a domain-specific language for defining mathematical expressions that generate audio waveforms in the Polysonix synthesizer. Expressions are stored as strings in WaveDefinition structures, tokenized, parsed into an abstract syntax tree (AST), compiled into bytecode, and executed by a virtual machine (VM) for real-time audio synthesis. This guide details the language's structure, operand symbols, and components, as used in `polysonix_wave.h`.

<details>
<summary>Table of Contents</summary>

- [1. Language Structure and Operands](#1-language-structure-and-operands)
  - [1.1. Language Structure](#11-language-structure)
  - [1.2. Parsing Hierarchy (Precedence, Highest to Lowest)](#12-parsing-hierarchy-precedence-highest-to-lowest)
  - [1.3. Operand Symbols](#13-operand-symbols)
  - [1.4. Variables and Parameters (Available within the script)](#14-variables-and-parameters-available-within-the-script)
  - [1.5. Constants](#15-constants)
  - [1.6. LFSR Type Constants (Integer values passed to LFSR functions)](#16-lfsr-type-constants-integer-values-passed-to-lfsr-functions)
  - [1.7. Functions](#17-functions)
  - [1.8. LFSR Functions (Detailed Behavior)](#18-lfsr-functions-detailed-behavior)
  - [1.9. Sigma Summation](#19-sigma-summation)
  - [1.10. Compilation and Execution](#110-compilation-and-execution)
  - [1.11. Constraints](#111-constraints)
  - [1.12. Wave Sequencing](#112-wave-sequencing)
  - [1.13. Example WaveDefinition (C struct storing the script string)](#113-example-wavedefinition-c-struct-storing-the-script-string)
- [2. Implementation Example](#2-implementation-example)
  - [2.1. Defining a Waveform Script](#21-defining-a-waveform-script)
  - [2.2. Creating a WaveDefinition Struct](#22-creating-a-wavedefinition-struct)
  - [2.3. Compiling the Waveform Script](#23-compiling-the-waveform-script)
  - [2.4. Using the Waveform in the Synthesizer](#24-using-the-waveform-in-the-synthesizer)
  - [2.5. Modulating the Waveform](#25-modulating-the-waveform)

</details>

## 1. Language Structure and Operands

### 1.1. Language Structure

The language follows a C-like mathematical expression grammar, parsed using recursive descent. Expressions are composed of:

- **Literals**: Floating-point numbers (e.g., 1.0, 0.5).
- **Variables**: x, FREQUENCY, MOD_A, MOD_B, MOD_C, RAND_OFFSET, and loop variables (e.g., k).
- **Constants**: PI, TWO_PI, PI_OVER_2, THREE_PI_OVER_2, E, LFSR type constants.
- **Functions**: Mathematical, utility, and LFSR functions (e.g., sin, sigma, lfsr_val).
- **Operators**: Arithmetic, unary, comparison, logical, and ternary.
- **Grouping**: Parentheses () for precedence and function arguments.
- **Commas**: Separate function arguments.

#### 1.1.1. Tokenizer
The tokenizer is the first stage of the compilation process. It takes the raw script as a string and breaks it down into a series of tokens. Each token represents a meaningful unit of the language, such as a number, a variable, an operator, or a function name. For example, the expression `sin(x + 0.5)` would be tokenized into `sin`, `(`, `x`, `+`, `0.5`, `)`.

#### 1.1.2. Parser and Abstract Syntax Tree (AST)
The parser takes the stream of tokens from the tokenizer and builds an Abstract Syntax Tree (AST). The AST is a tree-like data structure that represents the grammatical structure of the expression. The parser uses a recursive descent algorithm to handle the language's operator precedence and grouping rules. For example, the expression `2 + 3 * 4` would be parsed into an AST where the `+` operator is the root, with `2` as its left child and a subtree for `3 * 4` as its right child. This ensures that the multiplication is evaluated before the addition.

#### 1.1.3. Bytecode
Once the AST is built, the compiler traverses it and generates a sequence of bytecode instructions. Bytecode is a low-level, platform-independent representation of the script. It is designed to be executed efficiently by a virtual machine (VM). The bytecode for `polysonix` includes instructions for pushing constants and variables onto a stack, performing arithmetic and logical operations, calling functions, and managing control flow.

#### 1.1.4. Virtual Machine (VM)
The VM is a stack-based machine that executes the bytecode. It reads the bytecode instructions one by one and performs the corresponding operations. The VM maintains a stack for storing intermediate values and a set of registers for holding variables and parameters. The VM is designed to be small, fast, and efficient, making it suitable for real-time audio synthesis. The result of executing the bytecode is a single floating-point value, which represents the audio sample for the current phase `x`.

### 1.2. Parsing Hierarchy (Precedence, Highest to Lowest)

- **Primary**: Literals, variables, constants, functions, parenthesized expressions.
- **Unary**: +, -, !.
- **Factor**: *, /, %.
- **Term**: +, -.
- **Comparison**: <, >, <=, >=, ==, !=.
- **Logical AND**: &&.
- **Logical XOR**: ^.
- **Logical OR**: ||.
- **Conditional**: ? : (ternary).
- **Expression**: Full expression combining above.

**Example**: `"sin(x + 0.5 * MOD_A) + lfsr_noise(LFSR_8BIT, 2.0) * (x < PI ? 1.0 : -1.0)"`
Parsed as: `sin(primary) + lfsr_noise(primary, primary) * conditional(comparison ? primary : primary)`.

### 1.3. Operand Symbols
The language supports the following operators, listed with their symbols and roles:

**Arithmetic (Binary)**:
- `+` : Addition.
- `-` : Subtraction.
- `*` : Multiplication.
- `/` : Division.
- `%` : Modulus (remainder).

**Unary**:
- `+` : Positive (no-op).
- `-` : Negation.
- `!` : Logical NOT.

**Comparison**:
- `<` : Less than.
- `>` : Greater than.
- `<=` : Less than or equal to.
- `>=` : Greater than or equal to.
- `==` : Equal to.
- `!=` : Not equal to.

**Logical**:
- `&&` : Logical AND.
- `||` : Logical OR.
- `^` : Logical XOR (true if exactly one operand is true).

**Ternary**:
- `?` : Ternary condition (e.g., cond ? true_expr : false_expr).
- `:` : Ternary separator.

**Other Symbols**:
- `( )` : Parentheses for grouping and function calls.
- `,`   : Comma for function argument separation.

### 1.4. Variables and Parameters (Available within the script)
- `x`: Phase, typically normalized from 0 to 2*PI over one wave cycle.
- `FREQUENCY`: Current note frequency in Hertz (Hz).
- `MOD_A`, `MOD_B`, `MOD_C`: Modulation parameters, typically ranging from -1.0 to 1.0.
- `RAND_OFFSET`: A per-wave random value, typically from 0.0 to 1.0, constant for the duration of one wave generation.
- `k`: Default loop variable name for the sigma() summation function. Other names can be used.

### 1.5. Constants
- `PI`: 3.14159265358979323846.
- `TWO_PI`: 2 * PI.
- `PI_OVER_2`: PI / 2.
- `THREE_PI_OVER_2`: 3 * PI / 2.
- `E`: Base of natural logarithm (approx. 2.71828).

### 1.6. LFSR Type Constants (Integer values passed to LFSR functions)
- `LFSR_4BIT`: 0 (Period: 15)
- `LFSR_5BIT`: 1 (Period: 31)
- `LFSR_6BIT`: 2 (Period: 63)
- `LFSR_7BIT`: 3 (Period: 127)
- `LFSR_8BIT`: 4 (Period: 255)
- `LFSR_9BIT`: 5 (Period: 511)
- `LFSR_10BIT`: 6 (Period: 1023)
- `LFSR_11BIT`: 7 (Period: 2047)
- `LFSR_12BIT`: 8 (Period: 4095)
- `LFSR_13BIT`: 9 (Period: 8191)
- `LFSR_14BIT`: 10 (Period: 16383)
- `LFSR_15BIT`: 11 (Period: 32767)
- `LFSR_16BIT`: 12 (Period: 65535)
- `LFSR_17BIT`: 13 (Period: 131071)
- `LFSR_GALOIS`: 14 (Typically an alternative feedback topology, config matches LFSR_16BIT in current setup)
- `LFSR_FIBONACCI`: 15 (Standard feedback topology, config matches LFSR_16BIT in current setup)

### 1.7. Functions
The language supports these functions (name(arity)):
- `sin(1)`, `cos(1)`, `tan(1)`: Trigonometric functions (input in radians).
- `asin(1)`, `acos(1)`, `atan(1)`: Inverse trigonometric functions (result in radians).
- `abs(1)`: Absolute value.
- `tanh(1)`: Hyperbolic tangent.
- `exp(1)`: Exponential (e to the power of x).
- `log(1)`: Natural logarithm (ln(x)). Input must be > 0.
- `log10(1)`: Base-10 logarithm (log10(x)). Input must be > 0.
- `floor(1)`: Round down to the nearest integer.
- `ceil(1)`: Round up to the nearest integer.
- `min(2)`: Minimum of two values.
- `max(2)`: Maximum of two values.
- `sqrt(1)`: Square root. Input must be >= 0.
- `pow(2)`: Power (base, exponent).
- `rand()`: Returns a pseudo-random float between 0.0 and 1.0.
- `sigma(5)`: Summation (loop_var_name, start_val, end_val, step_val, expression_to_sum).
- `lfsr_val(3)`: LFSR bit value (type, position_norm, seed_norm).
- `lfsr_noise(2)`: LFSR bipolar noise (type, rate).
- `lfsr_clock(2)`: LFSR rhythmic pulses (type, density).

### 1.8. LFSR Functions (Detailed Behavior)
LFSR functions can operate in two modes depending on the C execution environment (VmParams setup):
1. **Precomputed Table Mode (Default)**: Uses pre-generated bit sequences for the specified LFSR type.
2. **Free-Running Mode**: Uses a stateful LFSR whose state is maintained in VmParams by the C caller. This mode is active if VmParams.lfsr_type matches the function's 'type' argument and VmParams.lfsr_state is non-zero.

- `lfsr_val(type, position_norm, seed_norm)`
  - Returns the current LFSR bit value (0.0 or 1.0).
  - **type**: LFSR type ID (e.g., LFSR_8BIT).
  - **Precomputed Mode**:
    - `position_norm`: Normalized position in the sequence (0.0 to <1.0, wraps).
    - `seed_norm`: Normalized offset added to position_norm (0.0 to <1.0, wraps).
  - **Free-Running Mode**:
    - Advances the free-running LFSR state for 'type' by one step.
    - Returns the new LSB.
    - 'position_norm' and 'seed_norm' arguments are popped from stack but IGNORED.

- `lfsr_noise(type, rate)`
  - Returns bipolar LFSR noise (-1.0 or 1.0).
  - **type**: LFSR type ID.
  - **Precomputed Mode**:
    - `rate`: Multiplier for how quickly the LFSR sequence is scanned relative to the main phase 'x'. (phase_for_lfsr = (x / TWO_PI) * rate)
  - **Free-Running Mode**:
    - Advances the free-running LFSR state for 'type' by one step.
    - Returns the new LSB converted to bipolar noise.
    - 'rate' argument is popped from stack but IGNORED (effective rate is 1 step per call).

- `lfsr_clock(type, density)`
  - Returns rhythmic clock pulses (0.0 or 1.0) from an LFSR.
  - **type**: LFSR type ID.
  - **density**: Threshold (0.0 to 1.0). A pulse (1.0) is generated if the LFSR bit is >= density.
  - **Precomputed Mode**:
    - Uses the main phase 'x' to determine position in the LFSR sequence.
  - **Free-Running Mode**:
    - Advances the free-running LFSR state for 'type' by one step.
    - Applies 'density' to the new LSB.

### 1.9. Sigma Summation
- **Syntax**: `sigma(k, start, end, step, expr)`
  - `k`: The name of the loop variable (e.g., 'k', 'i', 'n'). Must be a valid identifier.
  - `start`: The initial floating-point value of the loop variable (inclusive).
  - `end`: The final floating-point value of the loop variable. The loop continues as long as k <= end (for positive step) or k >= end (for negative step).
  - `step`: The floating-point increment (or decrement if negative) for the loop variable per iteration. Cannot be zero.
  - `expr`: The expression to be evaluated and summed. This expression can use the loop variable 'k'.

- **Example**: `"sigma(k, 1.0, 8.0, 1.0, sin(x*k)/k)"`
  - Sums `sin(x*k)/k` for k = 1.0, 2.0, ..., 8.0.

### 1.10. Compilation and Execution
- **Tokenizer**: Splits expressions into tokens (TOKEN_NUMBER, TOKEN_VARIABLE, etc.).
- **Parser**: Builds an Abstract Syntax Tree (AST) via functions like parseExpression, parseTerm, etc.
- **Compiler**: Translates the AST into a BytecodeChunk containing bytecode instructions, a constant pool, a string pool, and (for sigma) sub-chunks.
- **VM**: A stack-based virtual machine executes the bytecode. It receives runtime parameters via a VmParams struct, which includes:
    - x (current phase)
    - frequency (current note frequency)
    - modA, modB (modulation inputs)
    - rand_offset (per-wave random value)
    - lfsr_state, lfsr_type, lfsr_position, lfsr_seed (for managing free-running LFSR state by the C caller).
- **LFSR System**:
  - **Pre-computed Tables**: For each LFSR type, a bit sequence can be pre-generated and stored. In this mode, LFSR functions perform a lookup into these tables.
  - **Free-Running State**: Alternatively, the C code calling the VM can manage an LFSR's state (current value, type, position) within the VmParams struct. If an LFSR function call in the script matches the type configured for free-running mode in VmParams, the VM will advance this external state and use its output, potentially ignoring some script arguments (like position, seed, or rate for free-running LFSRs).
- **Output**: The VM's execution of the bytecode for a given set of VmParams results in a single floating-point value, typically representing an audio sample, which is then clamped to the range [-1.0, 1.0].

### 1.11. Constraints
- Max bytecode size per chunk: 1024 bytes.
- Max constants per chunk: 256 floats.
- Max VM stack depth: 512 floats.
- Max sigma sub-chunks per main chunk: 16.
- Max unique strings per chunk: 32.
- LFSR pre-computed table memory: Approximately 520KB for all defined types if fully initialized.

### 1.12. Wave Sequencing
(This section describes C-level structures, not the scripting language itself)
Sequences in `default_sequences` (defined in C) specify wave index, cycle count, and state flags
(e.g., `WSTA_SEQ_END`, `WSTA_SEQ_PITCH_SCALE`).

**Example**:
`{42, 1, WSTA_SEQ_RETRIGGER_ADSR}, {42, 1, WSTA_SEQ_MUTE}, ...`

### 1.13. Example WaveDefinition (C struct storing the script string)
`{ "LFSR Rhythm", "sin(x) * lfsr_clock(LFSR_8BIT, 0.5 + 0.3 * MOD_A) + 0.2 * lfsr_noise(LFSR_4BIT, 2.0 + MOD_B)" }`

## 2. Implementation Example

This section provides a practical, step-by-step example of how to define a waveform script and use it within the Polysonix engine.

### 2.1. Defining a Waveform Script
First, you need to define your waveform script as a C string. Let's create a simple sine wave with a modulated frequency:

```c
const char* my_wave_script = "sin(x + MOD_A * 0.1)";
```

This script defines a sine wave where the phase `x` is modulated by the `MOD_A` parameter.

### 2.2. Creating a WaveDefinition Struct
Next, you need to create a `WaveDefinition` struct to hold the script and a name for the waveform. This is typically done in an array of `WaveDefinition` structs.

```c
#include "polysonix_wave.h"

WaveDefinition default_waves[] = {
    // ... other wave definitions
    { "My Modulated Sine", my_wave_script },
    // ... other wave definitions
};
```

### 2.3. Compiling the Waveform Script
The Polysonix engine needs to compile the script into bytecode before it can be used. This is typically done at initialization time. The `polysonix_wave.h` header provides a function to compile all the wave definitions in the `default_waves` array.

```c
// In your initialization code:
for (int i = 0; i < sizeof(default_waves) / sizeof(WaveDefinition); ++i) {
    default_waves[i].compiled_bytecode = compile_bytecode(default_waves[i].expression);
}
```

### 2.4. Using the Waveform in the Synthesizer
Once the waveform is compiled, you can use it in your synthesizer by referencing its index in the `default_waves` array. For example, to play a note with your new waveform, you would call `PX_NoteOn` with the appropriate wave index.

```c
// Assuming "My Modulated Sine" is at index 42 in the default_waves array
int my_wave_index = 42;
PX_NoteOn(synth, 60, my_wave_index, 0); // Play MIDI note 60 with our new wave
```

### 2.5. Modulating the Waveform
You can modulate the `MOD_A` parameter in real-time to change the sound of the waveform. This can be done using LFOs or other modulation sources in the Polysonix engine. For example, you can route an LFO to `MOD_A` to create a vibrato effect.

```c
// Route LFO 0 to MOD_A
PX_SetLFOModAmount(synth, 0, PX_LFO_DEST_PARAM1, 1.0f);
```

This will cause the `MOD_A` parameter in your script to be driven by the output of LFO 0, creating a dynamic and evolving sound.
