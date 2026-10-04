#include <stdint.h>
#include <stdio.h>

static inline uint64_t ticks(void) {
#if defined(__x86_64__)
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi) : : "memory"); // memory: don't move loads across it
    return ((uint64_t)hi << 32) | lo;
#elif defined(__aarch64__)
    uint64_t v;
    __asm__ volatile("isb\n\tmrs %0, cntvct_el0" : "=r"(v) : : "memory"); // isb: don't let the read run early
    return v;
#else
    return 0;
#endif
}

static unsigned pop_loop(uint64_t x) {
    unsigned c = 0;
    for (int i = 0; i < 64; i++) c += (x >> i) & 1;
    return c;
}

static unsigned pop_kernighan(uint64_t x) {
    unsigned c = 0;
    for (; x; c++) x &= x - 1; // clears the lowest set bit: one iteration per 1-bit
    return c;
}

static unsigned pop_builtin(uint64_t x) { return (unsigned)__builtin_popcountll(x); }

static unsigned pop_asm(uint64_t x) {
#if defined(__x86_64__)
    uint64_t r;
    __asm__("popcntq %1, %0" : "=r"(r) : "rm"(x) : "cc");
    return (unsigned)r;
#elif defined(__aarch64__)
    uint32_t r;
    __asm__("fmov d0, %1\n\t"
            "cnt v0.8b, v0.8b\n\t" // count the bits in each of the 8 bytes
            "addv b0, v0.8b\n\t"   // add the 8 counts together
            "umov %w0, v0.b[0]"
            : "=r"(r)
            : "r"(x)
            : "v0");
    return r;
#else
    return pop_builtin(x);
#endif
}

enum { N = 1 << 20, RUNS = 7 };
static uint64_t values[N];

static void bench(const char* name, unsigned (*fn)(uint64_t)) {
    uint64_t best = UINT64_MAX;
    unsigned long long checksum = 0;
    for (int r = 0; r < RUNS; r++) {
        // Without this barrier, the optimizer sees that every run computes the same sum from the
        // same array, computes it ONCE outside the timing, and reports ~0 ticks. The "memory"
        // clobber says "values[] may have changed", so the work must really be redone each run.
        __asm__ volatile("" ::: "memory");
        uint64_t t0 = ticks();
        unsigned long long sum = 0;
        for (int i = 0; i < N; i++) sum += fn(values[i]);
        __asm__ volatile("" : "+r"(sum)); // "sum is used here": it must be fully computed BEFORE the next tick read
        uint64_t t = ticks() - t0;
        if (t < best) best = t;
        checksum = sum; // use the result, or the optimizer deletes the loop
    }
    printf("%-11s %8.2f ticks/call   (checksum %llu)\n", name, (double)best / N, checksum);
}

int main(void) {
    uint64_t s = 88172645463325252ull;
    for (int i = 0; i < N; i++) { // xorshift64
        s ^= s << 13;
        s ^= s >> 7;
        s ^= s << 17;
        values[i] = s;
    }
    bench("loop", pop_loop);
    bench("kernighan", pop_kernighan);
    bench("builtin", pop_builtin);
    bench("inline asm", pop_asm);
    printf("\nLessons (typical results on x86-64 with clang -O2):\n"
           " - the builtin wins: the compiler vectorizes it and processes several values at once\n"
           " - the inline asm is a black box to the optimizer, so it can't be vectorized or\n"
           "   unrolled, and it often LOSES to plain C\n"
           " - with -march=native the compiler even recognizes Kernighan's loop as a popcount\n"
           " - without the barriers in bench(), the optimizer hoists the work out of the timed\n"
           "   region and every result reads ~0 ticks. Always sanity-check a benchmark.\n");
    return 0;
}
