// GCC/Clang extended inline assembly: the basics.
//
//   __asm__ [volatile] ( "template" : outputs : inputs : clobbers );
//
// %0, %1 ... (or %[name]) in the template are replaced by the operands the compiler
// picked. Constraints tell it what kind of operand to provide:
//   "r" any register       "m" memory           "i" immediate constant
//   "=r" output (write-only)   "+r" read and write   "0" same location as operand 0
//   x86 specific: "a" = eax/rax, "b" = ebx, "c" = ecx, "d" = edx
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int asm_add(int a, int b) {
    int result;
#if defined(__x86_64__)
    // AT&T syntax: source first, destination last. "0"(a) puts a in the same register as result.
    __asm__("addl %2, %0" : "=r"(result) : "0"(a), "r"(b) : "cc"); // "cc": flags are modified
#elif defined(__aarch64__)
    // %w0 = the 32-bit view (w register) of operand 0. ARM syntax: destination first.
    __asm__("add %w0, %w1, %w2" : "=r"(result) : "r"(a), "r"(b));
#else
    result = a + b;
#endif
    return result;
}

// A cycle/tick counter. `volatile` stops the compiler from merging or hoisting it,
// since it has no inputs and would otherwise look like a pure function.
static uint64_t read_ticks(void) {
#if defined(__x86_64__)
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi)); // the result is in edx:eax
    return ((uint64_t)hi << 32) | lo;
#elif defined(__aarch64__)
    uint64_t v;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(v)); // read a system register
    return v;
#else
    return 0;
#endif
}

static void cpu_info(void) {
#if defined(__x86_64__)
    uint32_t regs[12];
    char brand[49] = {0};
    for (uint32_t i = 0; i < 3; i++) {
        uint32_t a, b, c, d;
        // CPUID: the leaf in eax, results in eax/ebx/ecx/edx. Leaves 0x80000002..4 hold the brand string.
        __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(0x80000002u + i), "c"(0));
        regs[i * 4 + 0] = a, regs[i * 4 + 1] = b, regs[i * 4 + 2] = c, regs[i * 4 + 3] = d;
    }
    memcpy(brand, regs, 48);
    printf("CPU (via cpuid): %s\n", brand);
#elif defined(__aarch64__)
    uint64_t freq;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(freq));
    printf("AArch64 generic timer frequency (via cntfrq_el0): %llu Hz\n", (unsigned long long)freq);
#endif
}

int main(void) {
    printf("asm_add(40, 2) = %d\n", asm_add(40, 2));
    cpu_info();

    uint64_t t0 = read_ticks();
    volatile uint64_t sink = 0;
    for (int i = 0; i < 1000000; i++) sink += (uint64_t)i;
    uint64_t t1 = read_ticks();
    printf("1M loop iterations took %llu ticks\n", (unsigned long long)(t1 - t0));
    printf("(x86 TSC ticks at a fixed rate near the base clock; ARM's counter usually runs at 24 MHz on Apple chips)\n");
    return 0;
}
