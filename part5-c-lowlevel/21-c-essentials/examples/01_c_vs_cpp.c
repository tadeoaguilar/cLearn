// The first differences a C++ programmer notices in C.
#include <stdbool.h> // bool, true, false (keywords only since C23)
#include <stddef.h>  // size_t, NULL
#include <stdint.h>  // int32_t, uint8_t, ...
#include <stdio.h>   // printf

// In C, an empty parameter list means "unspecified arguments" (pre-C23).
// Always write (void) for "no arguments".
static int answer(void) { return 42; }

// No overloading: give each function a distinct name.
static int max_int(int a, int b) { return a > b ? a : b; }
static double max_double(double a, double b) { return a > b ? a : b; }

// The idiomatic C integer constant is an enum; `const int` is not a constant expression.
enum { MAX_PLAYERS = 4 };

int main(void) {
    printf("answer() = %d\n", answer());
    printf("max_int(3, 7) = %d, max_double(2.5, 1.5) = %.1f\n", max_int(3, 7), max_double(2.5, 1.5));

    // printf formats must match the argument types exactly.
    size_t n = sizeof(int);
    long big = 1234567890L;
    unsigned char byte = 200;
    int32_t fixed = -5;
    printf("size_t %%zu: %zu | long %%ld: %ld | uchar %%u: %u | int32_t %%d: %d\n", n, big, byte, fixed);
    printf("pointer %%p: %p\n", (void*)&n);

    // A character literal is an int in C (in C++ it's a char).
    printf("sizeof('a') = %zu (C++ would say 1)\n", sizeof('a'));

    bool ready = true;
    printf("bool: %d (printed as an int; there is no %%b for bool)\n", ready);

    int scores[MAX_PLAYERS] = {10, 20}; // remaining elements are zeroed
    for (int i = 0; i < MAX_PLAYERS; i++) printf("scores[%d] = %d\n", i, scores[i]);

    // Integer promotion: uint8_t operands are promoted to int before arithmetic.
    uint8_t a = 200, b = 100;
    int sum = a + b;          // 300: computed in int
    uint8_t wrapped = a + b;  // 44: truncated when stored back into 8 bits
    printf("200 + 100 as int = %d, stored in uint8_t = %u\n", sum, wrapped);
    return 0;
}
