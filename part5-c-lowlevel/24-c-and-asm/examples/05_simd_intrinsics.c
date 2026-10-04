// Intrinsics: SIMD instructions from C without writing assembly.
//
// An intrinsic looks like a function but compiles to (usually) one instruction. The compiler
// still does register allocation and scheduling for you, so prefer intrinsics to inline asm
// whenever one exists.
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(__x86_64__)
#include <immintrin.h> // SSE/AVX: SSE2 is always available on x86-64
#elif defined(__aarch64__)
#include <arm_neon.h> // NEON is always available on AArch64
#endif

enum { N = 1 << 20, REPS = 200 };

static float sum_scalar(const float* a, size_t n) {
    float s = 0;
    // The compiler may NOT vectorize this at -O2: float addition isn't associative, and
    // vectorizing reorders the additions. Only -ffast-math gives it that permission.
    for (size_t i = 0; i < n; i++) s += a[i];
    return s;
}

static float sum_simd(const float* a, size_t n) {
    size_t i = 0;
#if defined(__x86_64__)
    __m128 acc = _mm_setzero_ps();                          // 4 floats, all 0
    for (; i + 4 <= n; i += 4) acc = _mm_add_ps(acc, _mm_loadu_ps(a + i)); // 4 additions per instruction
    float lanes[4];
    _mm_storeu_ps(lanes, acc);
    float s = lanes[0] + lanes[1] + lanes[2] + lanes[3];
#elif defined(__aarch64__)
    float32x4_t acc = vdupq_n_f32(0.0f);
    for (; i + 4 <= n; i += 4) acc = vaddq_f32(acc, vld1q_f32(a + i));
    float s = vaddvq_f32(acc); // horizontal add of the 4 lanes
#else
    float s = 0;
#endif
    for (; i < n; i++) s += a[i]; // leftover elements
    return s;
}

static double time_it(float (*fn)(const float*, size_t), const float* a, float* result) {
    clock_t start = clock();
    for (int r = 0; r < REPS; r++) *result = fn(a, N);
    return (double)(clock() - start) / CLOCKS_PER_SEC;
}

int main(void) {
    float* a = malloc(N * sizeof *a);
    if (!a) return 1;
    for (size_t i = 0; i < N; i++) a[i] = (float)(i % 100) * 0.01f;

    float r1, r2;
    double t1 = time_it(sum_scalar, a, &r1);
    double t2 = time_it(sum_simd, a, &r2);
    printf("scalar: sum=%.1f  %.3f s\n", r1, t1);
    printf("SIMD:   sum=%.1f  %.3f s  (%.1fx faster)\n", r2, t2, t2 > 0 ? t1 / t2 : 0.0);
    printf("The sums can differ slightly: the additions happen in a different order (rounding).\n");
    free(a);
    return 0;
}
