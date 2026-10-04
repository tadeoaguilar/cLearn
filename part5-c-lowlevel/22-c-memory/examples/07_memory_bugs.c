// Intentionally broken code. Build with AddressSanitizer and run one bug at a time:
//
//   cc -std=c17 -g -fsanitize=address,undefined -fno-omit-frame-pointer 07_memory_bugs.c -o bugs
//   ./bugs overflow | uaf | double | leak | stack | uninit
//
// Without arguments it only prints this help, so it's safe to run in scripts.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Hide pointers from the optimizer and static warnings, so the bugs really happen at runtime.
static void* volatile sink;
static void* launder(void* p) {
    sink = p;
    return sink;
}

static void heap_overflow(void) {
    int* a = malloc(4 * sizeof *a);
    if (!a) return;
    for (int i = 0; i <= 4; i++) a[i] = i; // off-by-one: writes a[4]
    printf("wrote a[4] (out of bounds)\n");
    free(a);
}

static void use_after_free(void) {
    char* name = malloc(16);
    if (!name) return;
    strcpy(name, "dangling");
    char* alias = launder(name);
    free(name);
    printf("reading freed memory: %c\n", alias[0]);
}

static void double_free(void) {
    void* p = malloc(32);
    void* alias = launder(p);
    free(p);
    free(alias);
}

static void leak(void) {
    for (int i = 0; i < 10; i++) {
        char* buf = malloc(100);
        if (!buf) return;
        snprintf(buf, 100, "message %d", i);
        launder(buf);
    } // 10 × 100 bytes leaked: nobody called free
    sink = NULL;
    printf("leaked 1000 bytes\n");
}

static int* stack_escape_helper(void) {
    int local = 42;
    return launder(&local); // returns the address of a dead stack variable
}
static void stack_use_after_return(void) {
    int* p = stack_escape_helper();
    printf("value from a dead stack frame: %d\n", *p);
}

static void uninitialized(void) {
    int* a = malloc(8 * sizeof *a);
    if (!a) return;
    int* b = launder(a);
    // ASan does NOT catch this; Valgrind or MemorySanitizer (clang on Linux, -fsanitize=memory) does.
    if (b[3] > 0) printf("uninitialized value was positive\n");
    else printf("uninitialized value was <= 0\n");
    free(a);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: %s overflow|uaf|double|leak|stack|uninit\n", argv[0]);
        printf("build with -fsanitize=address,undefined -g to see each bug diagnosed\n");
        printf("(stack-use-after-return also needs ASAN_OPTIONS=detect_stack_use_after_return=1)\n");
        return 0;
    }
    const char* which = argv[1];
    if (!strcmp(which, "overflow")) heap_overflow();
    else if (!strcmp(which, "uaf")) use_after_free();
    else if (!strcmp(which, "double")) double_free();
    else if (!strcmp(which, "leak")) leak();
    else if (!strcmp(which, "stack")) stack_use_after_return();
    else if (!strcmp(which, "uninit")) uninitialized();
    else printf("unknown bug '%s'\n", which);
    return 0;
}
