// Calling assembly functions from C, and C functions from assembly.
// build: also-compile 04_functions.S
//
//   cc -std=c17 -Wall 04_calling_asm.c 04_functions.S -o calling_asm && ./calling_asm
//
// To C, an assembly function is just a symbol. The prototype below is a PROMISE that
// the asm follows the platform's calling convention; the compiler can't check it.
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int64_t asm_add(int64_t a, int64_t b);
int64_t asm_sum_array(const int64_t* a, size_t n);
size_t asm_strlen(const char* s);
int64_t asm_max3(int64_t a, int64_t b, int64_t c);
int64_t asm_call_twice(int64_t (*fn)(int64_t), int64_t x);

static int64_t triple(int64_t x) { return x * 3; }
static int64_t square(int64_t x) { return x * x; }

int main(void) {
    printf("asm_add(40, 2) = %" PRId64 "\n", asm_add(40, 2));

    int64_t nums[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    printf("asm_sum_array(1..10) = %" PRId64 "\n", asm_sum_array(nums, 10));
    printf("asm_sum_array(empty) = %" PRId64 "\n", asm_sum_array(nums, 0));

    const char* words[] = {"", "x", "assembly", "hello, world"};
    for (int i = 0; i < 4; i++)
        printf("asm_strlen(\"%s\") = %zu (strlen %zu)\n", words[i], asm_strlen(words[i]), strlen(words[i]));

    printf("asm_max3(3, -7, 12) = %" PRId64 ", asm_max3(-1, -2, -3) = %" PRId64 "\n", asm_max3(3, -7, 12),
           asm_max3(-1, -2, -3));

    printf("asm_call_twice(triple, 5) = %" PRId64 "\n", asm_call_twice(triple, 5));
    printf("asm_call_twice(square, 3) = %" PRId64 "\n", asm_call_twice(square, 3));
    return 0;
}
