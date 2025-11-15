# Polysonix Wave Bytecode Compute Shader Analysis

This document provides a comprehensive analysis of the `polysonix_wave.comp` GLSL compute shader, designed to execute the Polysonix waveform scripting language bytecode on the GPU.

## 1. Overview

The `polysonix_wave.comp` shader is a single-workgroup compute shader that functions as a stack-based virtual machine (VM). It interprets a custom bytecode format, defined in `polysonix_wave.h`, to generate a single floating-point audio sample. This allows for offloading the computationally intensive part of audio synthesis from the CPU to the GPU.

The shader is designed to be dispatched once per required audio sample, with all necessary data (bytecode, constants, parameters) provided via Shader Storage Buffer Objects (SSBOs) and uniforms.

## 2. GLSL Implementation Details

### 2.1. VM State

The VM's state is managed using global variables within the shader:

- `float vm_stack[MAX_VM_STACK]`: A fixed-size array for the operand stack.
- `int stack_top`: The stack pointer, an index into `vm_stack`.
- `int ip`: The instruction pointer, an index into the bytecode buffer.
- `float loop_var_value`: Stores the current value of the loop variable during `OP_SIGMA_EXEC`.
- `bool is_in_sigma_body`: A flag to indicate when execution is inside a sigma loop body.

### 2.2. Execution Flow

The main execution logic is contained within the `execute_chunk(int start_ip)` function, which can be called recursively to handle `sigma` sub-expressions.

1.  **Initialization**: The `main()` function initializes the VM state (`stack_top`, `ip`) and calls `execute_chunk` with the offset of the main bytecode.
2.  **VM Loop**: `execute_chunk` enters a `for` loop that acts as the VM's fetch-decode-execute cycle. A generous loop limiter (1024 iterations) is used to prevent accidental infinite loops on the GPU.
3.  **Instruction Processing**: A `switch` statement handles each `OpCode`.
4.  **Result**: The loop terminates on `OP_HALT` or when the loop limit is reached. The final value on top of the stack is returned. The `main` function clamps this result to `[-1.0, 1.0]` and writes it to the output buffer.

## 3. Data Marshalling (C Host to GLSL Shader)

The C host application is responsible for preparing and binding several data buffers before dispatching the shader.

### 3.1. SSBOs (Shader Storage Buffer Objects)

-   **Binding 0: Bytecode Buffer (`uint`)**:
    -   A single, contiguous buffer containing the bytecode of the main expression followed immediately by the bytecode of all its `sigma` sub-chunks.
    -   The C host must "flatten" the `BytecodeChunk` structure. If a wave has a main chunk and two `sigma` sub-chunks, the buffer layout would be: `[main_chunk_bytes, sub_chunk_0_bytes, sub_chunk_1_bytes]`.
    -   Data is packed as `uint` to align with GLSL's `uint` array. The C host should copy the `uint8_t` bytecode into a `uint32_t` aligned buffer.

-   **Binding 1: Constants Buffer (`float`)**:
    -   A single buffer containing all floating-point constants from the main chunk and all sub-chunks.
    -   `OP_PUSH_CONST` uses an index into this buffer.

-   **Binding 2: LFSR Tables Buffer (`uint`)**:
    -   A tightly packed buffer containing the pre-computed bit sequences for all 16 LFSR types.
    -   Each bit sequence is stored as a series of packed bytes (8 bits per byte). The C host should copy the `uint8_t* bit_table` data for each LFSR type into this single large buffer.

-   **Binding 3: Output Buffer (`float`)**:
    -   A single-element buffer where the final computed sample will be written.

### 3.2. Uniforms

-   **`VmParams` (Uniform Block)**:
    -   Contains the runtime variables (`x`, `frequency`, `modA`, etc.) for the current sample being generated.
    -   This data is identical to the `VmParams` struct in the C code.

-   **`VmMetadata` (Uniform Block)**:
    -   Provides the shader with essential offsets and metadata to navigate the flattened SSBOs.
    -   `main_chunk_offset`: The starting index (in bytes) of the main bytecode chunk within the Bytecode Buffer.
    -   `sigma_offsets_*`: The starting indices for each of the 16 possible `sigma` sub-chunks.
    -   `lfsr_periods`: An array containing the period length for each of the 16 LFSR types.
    -   `lfsr_offsets`: An array containing the starting index (in bytes) for each LFSR's bit table within the LFSR Tables Buffer.

## 4. Opcode Implementation Analysis

### 4.1. Standard Opcodes

-   **Stack, Arithmetic, Logical, and Control Flow** opcodes (`OP_PUSH_*`, `OP_ADD`, `OP_CMP_*`, `OP_JUMP_*`) are implemented with straightforward GLSL equivalents. They directly manipulate the `vm_stack` and `ip` global variables.

### 4.2. `OP_CALL` (Function Calls)

-   A `switch` statement maps the `FunctionID` to the corresponding GLSL built-in function (e.g., `sin`, `cos`, `pow`).
-   Arguments are popped from the stack, the function is executed, and the result is pushed back.
-   **LFSR Functions**:
    -   `lfsr_val`, `lfsr_noise`, and `lfsr_clock` are implemented using the `lfsr_get_bit` helper function.
    -   This helper calculates the correct index into the LFSR Tables Buffer using the provided `lfsr_periods` and `lfsr_offsets` metadata, extracts the packed bit, and returns it as a float.

### 4.3. `OP_SIGMA_EXEC` (Summation)

This is the most complex operation, handled via recursion:

1.  The opcode reads the indices for the `start`, `end`, `step`, and `body` sub-chunks.
2.  It retrieves the bytecode offsets for these sub-chunks from the `VmMetadata` uniforms.
3.  It calls `execute_chunk()` for the `start`, `end`, and `step` chunks to get the loop parameters. The VM state (stack, IP) is implicitly saved and restored by the nature of GLSL function calls.
4.  It enters a `for` loop on the CPU side, iterating from `start_val` to `end_val`.
5.  Inside the loop:
    -   The current loop value (`k`) is stored in the `loop_var_value` global.
    -   The `is_in_sigma_body` flag is set to `true`.
    -   `execute_chunk()` is called for the `body` sub-chunk. The `OP_PUSH_LOOP_VAR` opcode within this execution will read from `loop_var_value`.
    -   The result is added to a running `sum`.
6.  After the loop, the `is_in_sigma_body` flag is cleared, and the final `sum` is pushed onto the stack.

## 5. Limitations and Design Choices

-   **No Free-Running LFSRs**: The C VM supports a "free-running" LFSR mode where the LFSR state is maintained in the `VmParams` struct and updated on each call. This is not implemented in the shader because uniforms are read-only, and managing this state would require an additional read-write SSBO, adding complexity. All LFSR functions in the shader operate in the pre-computed table lookup mode.
-   **Recursion for Sigma**: The use of GLSL function recursion for `OP_SIGMA_EXEC` is clean but relies on the driver supporting a sufficient recursion depth. For Polysonix's use case (no nested sigma), this is safe.
-   **Single Workgroup**: The shader is designed for a `1x1x1` workgroup. This simplifies the design as no synchronization is needed, but it means the C host must dispatch one compute call per sample. For generating entire waveforms at once, a different shader structure (e.g., one invocation per sample in a 1D workgroup) would be needed.

## 6. Integration Steps for C Host

1.  **Load and Compile Shader**: Load `polysonix_wave.comp` and compile it into a compute shader program.
2.  **Prepare Buffers**:
    -   For each `WaveDefinition` that needs to be run on the GPU:
        -   Compile it to a `BytecodeChunk`.
        -   "Flatten" the `BytecodeChunk` and its sub-chunks into the single Bytecode Buffer.
        -   Gather all constants into the Constants Buffer.
    -   Create the global LFSR Tables Buffer once during initialization.
3.  **Create SSBOs**: Create and populate the four required SSBOs on the GPU.
4.  **Set Uniforms**: Before dispatching, update the `VmParams` and `VmMetadata` uniform blocks with the data corresponding to the specific bytecode and sample being generated.
5.  **Dispatch**: Execute `glDispatchCompute(1, 1, 1)`.
6.  **Retrieve Result**: Use a memory barrier (`glMemoryBarrier`) and then read back the data from the Output Buffer.
