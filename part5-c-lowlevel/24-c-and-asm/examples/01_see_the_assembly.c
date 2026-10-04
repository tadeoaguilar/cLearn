// Small functions that are worth reading in assembly. Generate the listing with:
//
//   cc -std=c17 -O2 -S -fno-asynchronous-unwind-tables 01_see_the_assembly.c -o - | less
//   (add -masm=intel on x86 for Intel syntax; or paste this file into https://godbolt.org)
//
// Try -O0 vs -O2 and compare: at -O0 every variable lives on the stack; at -O2 they live in
// registers, loops get unrolled or vectorized, and some functions disappear entirely.
#include <stdint.h>
#include <stdio.h>

// x86-64: lea eax, [rdi + rsi]. One instruction computes the sum (lea is "address arithmetic").
int add(int a, int b) { return a + b; }

// Division by a constant becomes a multiplication by a "magic number" and shifts: no div instruction.
unsigned div_by_10(unsigned x) { return x / 10; }

// A branch-free max: look for cmov (x86) or csel (ARM).
long max_long(long a, long b) { return a > b ? a : b; }

// A loop: at -O2 clang may replace it with the closed form n*(n-1)/2!
uint64_t sum_to(uint64_t n) {
    uint64_t s = 0;
    for (uint64_t i = 0; i < n; i++) s += i;
    return s;
}

// A dense switch becomes a jump table (an array of addresses indexed by the value).
const char* day_name(int d) {
    switch (d) {
    case 0: return "Mon";
    case 1: return "Tue";
    case 2: return "Wed";
    case 3: return "Thu";
    case 4: return "Fri";
    case 5: return "Sat";
    case 6: return "Sun";
    default: return "?";
    }
}

// A small struct is returned in registers (rax:rdx on x86-64, x0:x1 on AArch64), not through memory.
typedef struct {
    long quot, rem;
} DivResult;
DivResult divmod(long a, long b) { return (DivResult){a / b, a % b}; }

int main(void) {
    printf("add(2, 3) = %d\n", add(2, 3));
    printf("div_by_10(12345) = %u\n", div_by_10(12345));
    printf("max_long(-4, 9) = %ld\n", max_long(-4, 9));
    printf("sum_to(100000) = %llu\n", (unsigned long long)sum_to(100000));
    printf("day_name(4) = %s\n", day_name(4));
    DivResult r = divmod(47, 5);
    printf("divmod(47, 5) = {%ld, %ld}\n", r.quot, r.rem);

    // A function's machine code is just bytes in memory (in the read-only, executable .text section).
    const unsigned char* code = (const unsigned char*)(uintptr_t)&add;
    printf("first bytes of add(): ");
    for (int i = 0; i < 8; i++) printf("%02x ", code[i]);
    printf("\n(disassemble them with: objdump -d a.out | grep -A6 '<_\\?add>:')\n");
#if defined(__x86_64__)
    printf("architecture: x86-64\n");
#elif defined(__aarch64__)
    printf("architecture: AArch64\n");
#endif
    return 0;
}
