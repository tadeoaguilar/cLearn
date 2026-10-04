// malloc, calloc, realloc, free and aligned_alloc, used correctly.
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Overflow-checked array allocation: what calloc does internally, without zeroing.
static void* malloc_array(size_t n, size_t size) {
    if (size != 0 && n > SIZE_MAX / size) return NULL; // n * size would wrap around
    return malloc(n * size);
}

// Grow a buffer safely: on failure the old block is untouched and still owned by the caller.
static int grow(int** arr, size_t* cap) {
    size_t new_cap = *cap ? *cap * 2 : 4;
    int* tmp = realloc(*arr, new_cap * sizeof **arr);
    if (!tmp) return -1;
    printf("  realloc %zu → %zu ints: %s\n", *cap, new_cap,
           *arr == NULL ? "first allocation" : tmp == *arr ? "grew in place" : "block MOVED (contents copied)");
    *arr = tmp;
    *cap = new_cap;
    return 0;
}

int main(void) {
    // malloc does NOT zero. Never read before writing (we only print the pointer here).
    int* m = malloc(4 * sizeof *m);
    int* c = calloc(4, sizeof *c); // calloc zeroes
    if (!m || !c) return 1;
    printf("calloc'd values: %d %d %d %d\n", c[0], c[1], c[2], c[3]);
    memset(m, 0, 4 * sizeof *m); // if you need zeros from malloc, ask for them
    free(m);
    free(c);

    // The overflow check matters: (SIZE_MAX/2 + 2) * 2 wraps to a tiny number.
    size_t huge = SIZE_MAX / 2 + 2;
    void* bad = malloc_array(huge, 2);
    printf("malloc_array(SIZE_MAX/2+2, 2) = %p (rejected instead of allocating 2 bytes)\n", bad);
    void* bad2 = calloc(huge, 2);
    printf("calloc(SIZE_MAX/2+2, 2)       = %p\n", bad2);

    printf("growing an array with realloc:\n");
    int* arr = NULL; // realloc(NULL, n) behaves like malloc(n)
    size_t cap = 0;
    for (int round = 0; round < 6; round++) {
        if (grow(&arr, &cap) != 0) {
            free(arr);
            return 1;
        }
        // Allocate something else in between, so realloc sometimes has to move.
        free(malloc(cap * 4));
    }
    for (size_t i = 0; i < cap; i++) arr[i] = (int)i;
    printf("arr[%zu] = %d\n", cap - 1, arr[cap - 1]);
    free(arr);
    arr = NULL; // a habit that turns use-after-free into an obvious NULL crash

    // Over-aligned memory: for SIMD (32/64 bytes) or cache-line-sized buffers.
    double* simd = aligned_alloc(64, 64 * sizeof *simd); // size must be a multiple of the alignment
    if (!simd) return 1;
    printf("aligned_alloc(64, ...) = %p → address %% 64 = %zu\n", (void*)simd, (size_t)((uintptr_t)simd % 64));
    printf("malloc's guaranteed alignment: alignof(max_align_t) = %zu\n", alignof(max_align_t));
    free(simd);
    return 0;
}
