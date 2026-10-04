// Print an address from each region of the process's memory.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static int bss_counter;            // .bss: zero-initialized global
static int data_limit = 10;        // .data: initialized global
static const char rodata_msg[] = "read-only";   // .rodata

static void show(const char* region, const void* addr) {
    printf("  %-28s %18p\n", region, addr);
}

static void deeper(int depth) {
    int local = depth;
    if (depth < 2) {
        deeper(depth + 1);
        return;
    }
    show("stack (3 calls deeper)", (void*)&local); // lower address: the stack grows down
}

int main(void) {
    int local = 0;
    int* small = malloc(sizeof *small);
    char* big = malloc(1 << 20); // 1 MB: usually served by mmap, far from the small blocks
    if (!small || !big) return 1;

    printf("Region                       Address (ASLR changes these every run)\n");
    show(".text   (function main)", (void*)(uintptr_t)&main);
    show(".rodata (const array)", (const void*)rodata_msg);
    show(".rodata (string literal)", (const void*)"hello");
    show(".data   (initialized global)", (void*)&data_limit);
    show(".bss    (zeroed global)", (void*)&bss_counter);
    show("heap    (small malloc)", (void*)small);
    show("heap    (1 MB malloc)", (void*)big);
    show("stack   (local in main)", (void*)&local);
    deeper(0);

    printf("\nbss_counter starts at %d, data_limit at %d\n", bss_counter, data_limit);
    free(big);
    free(small);
    return 0;
}
