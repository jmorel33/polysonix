#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>

#include "px_vm.h"

#ifndef fabsf
#define fabsf(x) ((x) < 0 ? -(x) : (x))
#endif

void test_zero() {
    printf("Testing vm_fast_tanh(0.0f)... ");
    float result = vm_fast_tanh(0.0f);
    assert(result == 0.0f);
    printf("Passed\n");
}

void test_symmetry() {
    printf("Testing symmetry... ");
    float values[] = {0.1f, 0.5f, 1.0f, 2.0f, 10.0f};
    for (int i = 0; i < 5; i++) {
        float x = values[i];
        float pos = vm_fast_tanh(x);
        float neg = vm_fast_tanh(-x);
        assert(fabsf(pos + neg) < 1e-6f);
    }
    printf("Passed\n");
}

void test_accuracy() {
    printf("Testing accuracy against tanhf... ");
    // The approximation is Padé [2/2] for small values.
    // Let's check for x in [-1.0, 1.0]
    for (float x = -1.0f; x <= 1.0f; x += 0.1f) {
        float fast = vm_fast_tanh(x);
        float real = tanhf(x);
        float diff = fabsf(fast - real);
        // Padé [2/2] for tanh(x) is (x + x^3/27) / (1 + x^2/3)?
        // No, the code says: y = x * (27.0f + x2) / fmaf(9.0f, x2, 27.0f);
        // y = (27x + x^3) / (9x^2 + 27) = (x + x^3/27) / (1 + x^2/3)
        // Standard tanh expansion: x - x^3/3 + 2x^5/15...
        // Padé [2/2] should be reasonably accurate for small x.
        // For x=1, tanh(1) approx 0.76159
        // fast = (27 + 1) / (9 + 27) = 28 / 36 = 7/9 approx 0.77777
        // Diff is ~0.016. So 0.02 is a safe epsilon for x=1.
        assert(diff < 0.02f);
    }
    printf("Passed\n");
}

void test_clamping() {
    printf("Testing clamping for large values... ");
    float large_values[] = {5.0f, 10.0f, 100.0f, -5.0f, -10.0f, -100.0f};
    for (int i = 0; i < 6; i++) {
        float x = large_values[i];
        float result = vm_fast_tanh(x);
        assert(result >= -1.0f && result <= 1.0f);
        if (x > 5.0f) {
            // Asymptotically it goes to x/9 before clamping.
            // tanhf(5) is 0.9999, so fast_tanh should be 1.0 due to clamping if x/9 > 1
            // 10/9 > 1, so for x=10 it must be 1.0
            if (x > 9.0f) assert(result == 1.0f);
        }
        if (x < -9.0f) {
            assert(result == -1.0f);
        }
    }
    printf("Passed\n");
}

int main() {
    printf("Running tests for vm_fast_tanh...\n");
    test_zero();
    test_symmetry();
    test_accuracy();
    test_clamping();
    printf("All tests for vm_fast_tanh passed!\n");
    return 0;
}
