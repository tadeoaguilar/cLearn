#include <stdint.h>
#include <stdio.h>

static uint32_t rotl32_c(uint32_t x, unsigned n) {
    n &= 31;
    return (x << n) | (x >> (-n & 31)); // -n & 31 avoids the UB of shifting by 32 when n == 0
}

static uint32_t rotl32_asm(uint32_t x, unsigned n) {
#if defined(__x86_64__)
    __asm__("roll %%cl, %0" : "+r"(x) : "c"(n) : "cc"); // %% = a literal % in the template
#elif defined(__aarch64__)
    __asm__("ror %w0, %w0, %w1" : "+r"(x) : "r"((32 - (n & 31)) & 31));
#else
    x = rotl32_c(x, n);
#endif
    return x;
}

int main(void) {
    uint32_t rng = 2463534242u;
    int mismatches = 0;
    for (int i = 0; i < 100000; i++) {
        rng ^= rng << 13; // xorshift32 pseudo-random generator
        rng ^= rng >> 17;
        rng ^= rng << 5;
        unsigned n = i % 33; // includes 0 and 32
        if (rotl32_asm(rng, n) != rotl32_c(rng, n)) mismatches++;
    }
    printf("rotl32(0x80000001, 1) = 0x%08X\n", rotl32_asm(0x80000001u, 1));
    printf("rotl32(0x12345678, 8) = 0x%08X\n", rotl32_asm(0x12345678u, 8));
    printf("rotl32(0xDEADBEEF, 0) = 0x%08X\n", rotl32_asm(0xDEADBEEFu, 0));
    printf("100000 random checks: %d mismatches\n", mismatches);
    printf("Answer: at -O2 the C version compiles to a single rol (x86) / ror (ARM) instruction.\n"
           "The compiler recognizes the idiom, so the asm buys nothing. That's the usual outcome.\n");
    return mismatches != 0;
}
