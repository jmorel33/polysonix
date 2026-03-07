#include "px_vm.h"
#include <stdio.h>
#include <math.h>

#define EPSILON 1e-6f

int main() {
    printf("Testing OP_SELECT...\n");
    // Add logic here to compile and test an expression using select
    BytecodeChunk* chunk = compile_expression_to_bytecode("select(0.3, 1, 2, 3)");
    if (!chunk) {
        printf("Failed to compile select expression.\n");
        return 1;
    }
    VmParams params = {0};
    float result = execute_bytecode(chunk, &params);
    if (fabs(result - 1.0f) > EPSILON) {
        printf("Test failed. Expected 1.0, got %f\n", result);
        return 1;
    }

    // Out of bounds
    chunk = compile_expression_to_bytecode("select(1.5, 1, 2)");
    result = execute_bytecode(chunk, &params);
    if (fabs(result - 2.0f) > EPSILON) {
        printf("Test failed (upper bounds). Expected 2.0, got %f\n", result);
        return 1;
    }

    // Out of bounds lower
    chunk = compile_expression_to_bytecode("select(-0.5, 3, 4)");
    result = execute_bytecode(chunk, &params);
    if (fabs(result - 3.0f) > EPSILON) {
        printf("Test failed (lower bounds). Expected 3.0, got %f\n", result);
        return 1;
    }

    printf("OP_SELECT works perfectly!\n");
    return 0;
}
