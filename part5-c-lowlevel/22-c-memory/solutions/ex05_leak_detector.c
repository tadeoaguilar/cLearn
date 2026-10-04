#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void* ptr;
    size_t size;
    const char* file;
    int line;
} Allocation;

enum { MAX_TRACKED = 1024 };
static Allocation live[MAX_TRACKED];
static size_t live_count;
static size_t total_allocs, total_bytes;

static void* dbg_malloc(size_t size, const char* file, int line) {
    void* p = malloc(size);
    if (!p) return NULL;
    if (live_count == MAX_TRACKED) {
        fprintf(stderr, "[leakcheck] table full, %s:%d not tracked\n", file, line);
        return p;
    }
    live[live_count++] = (Allocation){p, size, file, line};
    total_allocs++;
    total_bytes += size;
    return p;
}

static void dbg_free(void* p, const char* file, int line) {
    if (!p) return;
    for (size_t i = 0; i < live_count; i++) {
        if (live[i].ptr == p) {
            live[i] = live[--live_count]; // swap-remove: order doesn't matter
            free(p);
            return;
        }
    }
    // We don't call free() here: freeing an unknown pointer would corrupt the real heap.
    printf("[leakcheck] %s:%d: FREE of untracked pointer %p (double free?)\n", file, line, p);
}

static void report_leaks(void) {
    size_t bytes = 0;
    for (size_t i = 0; i < live_count; i++) bytes += live[i].size;
    printf("[leakcheck] %zu allocations (%zu bytes) total; %zu still live (%zu bytes)\n", total_allocs, total_bytes,
           live_count, bytes);
    for (size_t i = 0; i < live_count; i++)
        printf("[leakcheck]   LEAK %zu bytes allocated at %s:%d\n", live[i].size, live[i].file, live[i].line);
}

#define MALLOC(n) dbg_malloc((n), __FILE__, __LINE__)
#define FREE(p) dbg_free((p), __FILE__, __LINE__)

static char* make_greeting(const char* name) {
    size_t n = strlen(name) + 8;
    char* s = MALLOC(n);
    if (s) snprintf(s, n, "Hello, %s", name);
    return s;
}

int main(void) {
    atexit(report_leaks); // runs after main returns

    char* g1 = make_greeting("Ada");
    char* g2 = make_greeting("Linus");
    int* numbers = MALLOC(10 * sizeof *numbers);
    if (g1) printf("%s\n", g1);
    if (g2) printf("%s\n", g2);

    FREE(g1);
    FREE(numbers);
    FREE(g1); // bug: double free, caught
    // g2 is never freed: reported at exit with the line inside make_greeting
    return 0;
}
