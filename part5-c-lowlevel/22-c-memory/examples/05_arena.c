// An arena (bump) allocator: allocate fast, free everything at once.
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char* base;
    size_t cap;
    size_t offset; // everything before this is in use
    size_t peak;   // high-water mark, handy for tuning the size
} Arena;

static int arena_init(Arena* a, size_t cap) {
    a->base = malloc(cap); // ONE malloc for the arena's whole lifetime
    a->cap = a->base ? cap : 0;
    a->offset = a->peak = 0;
    return a->base ? 0 : -1;
}

static void arena_destroy(Arena* a) {
    free(a->base);
    *a = (Arena){0};
}

// align must be a power of two.
static void* arena_alloc_aligned(Arena* a, size_t size, size_t align) {
    uintptr_t current = (uintptr_t)a->base + a->offset;
    uintptr_t aligned = (current + (align - 1)) & ~(uintptr_t)(align - 1); // round up
    size_t new_offset = (size_t)(aligned - (uintptr_t)a->base) + size;
    if (new_offset > a->cap) return NULL; // out of space (a real arena might chain a new block)
    a->offset = new_offset;
    if (a->offset > a->peak) a->peak = a->offset;
    return (void*)aligned;
}

// Allocate `count` objects of type T with the right alignment.
#define ARENA_NEW(a, T, count) ((T*)arena_alloc_aligned((a), sizeof(T) * (count), alignof(T)))

static void arena_reset(Arena* a) { a->offset = 0; } // "free" everything in O(1)

// Temporary scope: remember the offset and roll back to it later (a stack of allocations).
typedef struct {
    Arena* arena;
    size_t saved;
} ArenaMark;
static ArenaMark arena_mark(Arena* a) { return (ArenaMark){a, a->offset}; }
static void arena_rollback(ArenaMark m) { m.arena->offset = m.saved; }

static char* arena_strdup(Arena* a, const char* s) {
    size_t n = strlen(s) + 1;
    char* p = arena_alloc_aligned(a, n, 1);
    if (p) memcpy(p, s, n);
    return p;
}

typedef struct {
    float x, y, vx, vy;
} Particle;

int main(void) {
    Arena frame;
    if (arena_init(&frame, 64 * 1024) != 0) return 1;

    // Simulate 3 game frames. Every frame allocates freely and the reset frees it all.
    for (int f = 0; f < 3; f++) {
        int n = 100 * (f + 1);
        Particle* ps = ARENA_NEW(&frame, Particle, n);
        char* label = arena_strdup(&frame, "frame-label");
        double* weights = ARENA_NEW(&frame, double, 10);
        if (!ps || !label || !weights) {
            printf("arena full!\n");
            break;
        }
        for (int i = 0; i < n; i++) ps[i] = (Particle){(float)i, 0, 1, 1};

        // Scratch memory that only lives for this block:
        ArenaMark m = arena_mark(&frame);
        char* tmp = ARENA_NEW(&frame, char, 4096);
        (void)tmp;
        printf("frame %d: %d particles, used %zu bytes (+4096 scratch)", f, n, m.saved);
        arena_rollback(m);
        printf(" → %zu after rollback\n", frame.offset);

        printf("  alignment: particles %% %zu = %zu, doubles %% 8 = %zu\n", alignof(Particle),
               (size_t)((uintptr_t)ps % alignof(Particle)), (size_t)((uintptr_t)weights % 8));
        arena_reset(&frame); // end of frame: free EVERYTHING with one assignment
    }
    printf("peak usage: %zu of %zu bytes\n", frame.peak, frame.cap);

    // Overflow behaviour
    void* too_big = arena_alloc_aligned(&frame, frame.cap + 1, 1);
    printf("allocating more than capacity returns %p\n", too_big);

    arena_destroy(&frame);
    return 0;
}
