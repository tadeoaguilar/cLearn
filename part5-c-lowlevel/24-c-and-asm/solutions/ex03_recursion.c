// build: also-compile ex03_recursion.S
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

uint64_t asm_fib(unsigned n);
uint64_t asm_factorial(unsigned n);

int main(void) {
    printf("fib:");
    for (unsigned i = 0; i <= 15; i++) printf(" %" PRIu64, asm_fib(i));
    printf("\nfib(90) = %" PRIu64 " (the largest that fits easily in 64 bits)\n", asm_fib(90));

    uint64_t expected = 1;
    int ok = 1;
    for (unsigned n = 0; n <= 20; n++) { // 20! is the largest factorial that fits in uint64_t
        if (n > 1) expected *= n;
        ok &= asm_factorial(n) == expected;
    }
    printf("asm_factorial(10) = %" PRIu64 ", asm_factorial(20) = %" PRIu64 "\n", asm_factorial(10), asm_factorial(20));
    printf("0..20 all correct: %s\n", ok ? "yes" : "NO");
    return !ok;
}
