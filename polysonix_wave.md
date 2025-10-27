# Polysonix Waveform Scripting Language
   (c) 2025 Jacques Morel

## Polysonix Waveform Scripting Language: Structure and Operands

### Overview:

The Polysonix Waveform Scripting Language is a domain-specific language for defining mathematical expressions that generate audio waveforms in the Polysonix synthesizer.
Expressions are stored as strings in WaveDefinition structures, tokenized, parsed into an abstract syntax tree (AST), compiled into bytecode, and executed by a virtual
machine (VM) for real-time audio synthesis. This comment details the language's structure, operand symbols, and components, as used in polysonix_wave.h.

### Language Structure:

The language follows a C-like mathematical expression grammar, parsed using recursive descent. Expressions are composed of:

- **Literals**: Floating-point numbers (e.g., 1.0, 0.5).
- **Variables**: x, FREQUENCY, MOD_A, MOD_B, MOD_C, RAND_OFFSET, and loop variables (e.g., k).
- **Constants**: PI, TWO_PI, PI_OVER_2, THREE_PI_OVER_2, E, LFSR type constants.
- **Functions**: Mathematical, utility, and LFSR functions (e.g., sin, sigma, lfsr_val).
- **Operators**: Arithmetic, unary, comparison, logical, and ternary.
- **Grouping**: Parentheses () for precedence and function arguments.
- **Commas**: Separate function arguments.

### Parsing Hierarchy (Precedence, Highest to Lowest):

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

### Operand Symbols:
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

### Variables and Parameters (Available within the script):
- `x`: Phase, typically normalized from 0 to 2*PI over one wave cycle.
- `FREQUENCY`: Current note frequency in Hertz (Hz).
- `MOD_A`, `MOD_B`, `MOD_C`: Modulation parameters, typically ranging from -1.0 to 1.0.
- `RAND_OFFSET`: A per-wave random value, typically from 0.0 to 1.0, constant for the duration of one wave generation.
- `k`: Default loop variable name for the sigma() summation function. Other names can be used.

### Constants:
- `PI`: 3.14159265358979323846.
- `TWO_PI`: 2 * PI.
- `PI_OVER_2`: PI / 2.
- `THREE_PI_OVER_2`: 3 * PI / 2.
- `E`: Base of natural logarithm (approx. 2.71828).

### LFSR Type Constants (Integer values passed to LFSR functions):
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

### Functions:
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

### LFSR Functions (Detailed Behavior):
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

### Sigma Summation:
- **Syntax**: `sigma(k, start, end, step, expr)`
  - `k`: The name of the loop variable (e.g., 'k', 'i', 'n'). Must be a valid identifier.
  - `start`: The initial floating-point value of the loop variable (inclusive).
  - `end`: The final floating-point value of the loop variable. The loop continues as long as k <= end (for positive step) or k >= end (for negative step).
  - `step`: The floating-point increment (or decrement if negative) for the loop variable per iteration. Cannot be zero.
  - `expr`: The expression to be evaluated and summed. This expression can use the loop variable 'k'.

- **Example**: `"sigma(k, 1.0, 8.0, 1.0, sin(x*k)/k)"`
  - Sums `sin(x*k)/k` for k = 1.0, 2.0, ..., 8.0.

### Compilation and Execution:
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

### Constraints:
- Max bytecode size per chunk: 1024 bytes.
- Max constants per chunk: 256 floats.
- Max VM stack depth: 512 floats.
- Max sigma sub-chunks per main chunk: 16.
- Max unique strings per chunk: 32.
- LFSR pre-computed table memory: Approximately 520KB for all defined types if fully initialized.

### Wave Sequencing:
(This section describes C-level structures, not the scripting language itself)
Sequences in `default_sequences` (defined in C) specify wave index, cycle count, and state flags
(e.g., `WSTA_SEQ_END`, `WSTA_SEQ_PITCH_SCALE`).

**Example**:
`{42, 1, WSTA_SEQ_RETRIGGER_ADSR}, {42, 1, WSTA_SEQ_MUTE}, ...`

### Example WaveDefinition (C struct storing the script string):
`{ "LFSR Rhythm", "sin(x) * lfsr_clock(LFSR_8BIT, 0.5 + 0.3 * MOD_A) + 0.2 * lfsr_noise(LFSR_4BIT, 2.0 + MOD_B)" }`
