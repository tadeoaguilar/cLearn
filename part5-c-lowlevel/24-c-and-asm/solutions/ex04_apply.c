// build: also-compile ex04_apply.S
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void asm_apply(int64_t* arr, size_t n, int64_t (*fn)(int64_t));

static int calls;
static int64_t noisy_square(int64_t x) {
    calls++;
    // printf is a big function that uses (and destroys) all the caller-saved registers,
    // and it also crashes on a misaligned stack. A good stress test for the asm.
    printf("  callback(%" PRId64 ") with printf inside\n", x);
    return x * x;
}
static int64_t negate(int64_t x) { return -x; }

int main(void) {
    int64_t data[] = {1, 2, 3, 4, 5};
    asm_apply(data, 5, noisy_square);
    printf("after square:");
    for (int i = 0; i < 5; i++) printf(" %" PRId64, data[i]);
    asm_apply(data, 5, negate);
    printf("\nafter negate:");
    for (int i = 0; i < 5; i++) printf(" %" PRId64, data[i]);
    asm_apply(data, 0, negate); // must not call fn at all
    printf("\ncallback calls: %d (expected 5)\n", calls);
    return calls != 5;
}
