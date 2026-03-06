#define DSP_MATH_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "dsp_math.h"

#define NUM_ITERATIONS 10000000

double get_time_in_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main() {
    InitFastDSP();

    FILE *f = fopen("benchmark_report.txt", "w");
    if (!f) {
        printf("Failed to open report file.\n");
        return 1;
    }

    fprintf(f, "Polysonix Math Benchmark Report\n");
    fprintf(f, "===============================\n");
    fprintf(f, "Iterations per test: %d\n\n", NUM_ITERATIONS);

    // Create some input data to avoid compiler optimizing away the loops
    float* inputs = (float*)malloc(NUM_ITERATIONS * sizeof(float));
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        inputs[i] = ((float)rand() / RAND_MAX) * M_PI * 2.0f - M_PI; // -PI to PI
    }

    float volatile sum = 0.0f; // Volatile to prevent optimization
    double start, end, t_std, t_fast;

    // --- sin ---
    sum = 0.0f;
    start = get_time_in_seconds();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        sum += sinf(inputs[i]);
    }
    end = get_time_in_seconds();
    t_std = end - start;

    sum = 0.0f;
    start = get_time_in_seconds();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        sum += fastsin(inputs[i]);
    }
    end = get_time_in_seconds();
    t_fast = end - start;

    fprintf(f, "sinf()     : %.5f seconds\n", t_std);
    fprintf(f, "fastsin()  : %.5f seconds (%.2fx speedup)\n\n", t_fast, t_std / t_fast);

    // --- cos ---
    sum = 0.0f;
    start = get_time_in_seconds();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        sum += cosf(inputs[i]);
    }
    end = get_time_in_seconds();
    t_std = end - start;

    sum = 0.0f;
    start = get_time_in_seconds();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        sum += fastcos(inputs[i]);
    }
    end = get_time_in_seconds();
    t_fast = end - start;

    fprintf(f, "cosf()     : %.5f seconds\n", t_std);
    fprintf(f, "fastcos()  : %.5f seconds (%.2fx speedup)\n\n", t_fast, t_std / t_fast);

    // --- tan ---
    sum = 0.0f;
    start = get_time_in_seconds();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        sum += tanf(inputs[i]);
    }
    end = get_time_in_seconds();
    t_std = end - start;

    sum = 0.0f;
    start = get_time_in_seconds();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        sum += fasttan(inputs[i]);
    }
    end = get_time_in_seconds();
    t_fast = end - start;

    fprintf(f, "tanf()     : %.5f seconds\n", t_std);
    fprintf(f, "fasttan()  : %.5f seconds (%.2fx speedup)\n\n", t_fast, t_std / t_fast);

    // --- asin ---
    // Rescale inputs for asin/acos to [-1, 1]
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        inputs[i] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
    }

    sum = 0.0f;
    start = get_time_in_seconds();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        sum += asinf(inputs[i]);
    }
    end = get_time_in_seconds();
    t_std = end - start;

    sum = 0.0f;
    start = get_time_in_seconds();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        sum += fastasin(inputs[i]);
    }
    end = get_time_in_seconds();
    t_fast = end - start;

    fprintf(f, "asinf()    : %.5f seconds\n", t_std);
    fprintf(f, "fastasin() : %.5f seconds (%.2fx speedup)\n\n", t_fast, t_std / t_fast);

    // --- acos ---
    sum = 0.0f;
    start = get_time_in_seconds();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        sum += acosf(inputs[i]);
    }
    end = get_time_in_seconds();
    t_std = end - start;

    sum = 0.0f;
    start = get_time_in_seconds();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        sum += fastacos(inputs[i]);
    }
    end = get_time_in_seconds();
    t_fast = end - start;

    fprintf(f, "acosf()    : %.5f seconds\n", t_std);
    fprintf(f, "fastacos() : %.5f seconds (%.2fx speedup)\n\n", t_fast, t_std / t_fast);

    // --- atan ---
    // Rescale inputs for atan back to general
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        inputs[i] = ((float)rand() / RAND_MAX) * 20.0f - 10.0f; // -10 to 10
    }

    sum = 0.0f;
    start = get_time_in_seconds();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        sum += atanf(inputs[i]);
    }
    end = get_time_in_seconds();
    t_std = end - start;

    sum = 0.0f;
    start = get_time_in_seconds();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        sum += fastatan(inputs[i]);
    }
    end = get_time_in_seconds();
    t_fast = end - start;

    fprintf(f, "atanf()    : %.5f seconds\n", t_std);
    fprintf(f, "fastatan() : %.5f seconds (%.2fx speedup)\n\n", t_fast, t_std / t_fast);


#if defined(PX_USE_SSE41) && defined(__SSE4_1__)
    fprintf(f, "--- SSE4.1 Paths ---\n\n");
    __m128 sum_sse = _mm_setzero_ps();

    // Create aligned float arrays
    float inputs_sse[4] __attribute__((aligned(16)));

    start = get_time_in_seconds();
    for (int i = 0; i < NUM_ITERATIONS; i += 4) {
        __m128 val = _mm_loadu_ps(&inputs[i]);
        sum_sse = _mm_add_ps(sum_sse, fastsin_sse(val));
    }
    end = get_time_in_seconds();
    t_fast = end - start;

    fprintf(f, "fastsin_sse() : %.5f seconds\n", t_fast);

    sum_sse = _mm_setzero_ps();
    start = get_time_in_seconds();
    for (int i = 0; i < NUM_ITERATIONS; i += 4) {
        __m128 val = _mm_loadu_ps(&inputs[i]);
        sum_sse = _mm_add_ps(sum_sse, fastcos_sse(val));
    }
    end = get_time_in_seconds();
    t_fast = end - start;

    fprintf(f, "fastcos_sse() : %.5f seconds\n\n", t_fast);

#endif

    fclose(f);
    free(inputs);
    FreeFastDSP();

    printf("Benchmark complete. Results written to benchmark_report.txt\n");
    return 0;
}
