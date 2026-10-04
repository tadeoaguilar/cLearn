// A first-fit allocator with splitting and two-way coalescing over a static heap.
//
// Heap layout: blocks laid end to end, each one a 16-byte header followed by its payload.
//
//   ┌────────┬──────────┬────────┬────────────────┬────────┬─────┐
//   │ header │ payload  │ header │ payload        │ header │ ... │
//   └────────┴──────────┴────────┴────────────────┴────────┴─────┘
//
// Payload sizes are multiples of 16, so the lowest bit of `size` is always 0 and
// we can borrow it as the "free" flag. Real allocators (dlmalloc, glibc) use the same trick.
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { HEAP_SIZE = 64 * 1024, ALIGN = 16, MIN_PAYLOAD = 16 };

typedef struct {
    size_t size_and_flag; // payload size | 1 if free
    size_t prev_size;     // payload size of the previous block (0 for the first): enables backward merging
} Header;
_Static_assert(sizeof(Header) == ALIGN, "header must preserve payload alignment");

static alignas(ALIGN) unsigned char heap[HEAP_SIZE];
static bool initialized;

static size_t block_size(const Header* h) { return h->size_and_flag & ~(size_t)1; }
static bool is_free(const Header* h) { return h->size_and_flag & 1; }
static void set_block(Header* h, size_t size, bool free) { h->size_and_flag = size | (free ? 1 : 0); }

static Header* next_block(Header* h) {
    unsigned char* n = (unsigned char*)(h + 1) + block_size(h);
    return n < heap + HEAP_SIZE ? (Header*)n : NULL;
}
static Header* prev_block(Header* h) {
    if ((unsigned char*)h == heap) return NULL;
    return (Header*)((unsigned char*)h - h->prev_size - sizeof(Header));
}
static void fix_next_prev_size(Header* h) {
    Header* n = next_block(h);
    if (n) n->prev_size = block_size(h);
}

static void heap_init(void) {
    Header* h = (Header*)heap;
    set_block(h, HEAP_SIZE - sizeof(Header), true);
    h->prev_size = 0;
    initialized = true;
}

void* my_malloc(size_t size) {
    if (!initialized) heap_init();
    if (size == 0 || size > HEAP_SIZE) return NULL;
    size = (size + ALIGN - 1) & ~(size_t)(ALIGN - 1);

    for (Header* h = (Header*)heap; h; h = next_block(h)) {
        if (!is_free(h) || block_size(h) < size) continue; // first fit
        size_t remainder = block_size(h) - size;
        if (remainder >= sizeof(Header) + MIN_PAYLOAD) { // split off the unused tail
            set_block(h, size, false);
            Header* rest = next_block(h);
            set_block(rest, remainder - sizeof(Header), true);
            rest->prev_size = size;
            fix_next_prev_size(rest);
        } else {
            set_block(h, block_size(h), false); // use the whole block
        }
        return h + 1; // the payload starts right after the header
    }
    return NULL; // no block big enough: out of memory (or too fragmented)
}

void my_free(void* p) {
    if (!p) return;
    Header* h = (Header*)p - 1;
    set_block(h, block_size(h), true);
    Header* next = next_block(h);
    if (next && is_free(next)) // merge forward: absorb the next block and its header
        set_block(h, block_size(h) + sizeof(Header) + block_size(next), true);
    Header* prev = prev_block(h);
    if (prev && is_free(prev)) { // merge backward: the previous block absorbs us
        set_block(prev, block_size(prev) + sizeof(Header) + block_size(h), true);
        h = prev;
    }
    fix_next_prev_size(h);
}

static void heap_dump(const char* title) {
    printf("%s\n", title);
    for (Header* h = (Header*)heap; h; h = next_block(h))
        printf("  @%5zu  %-4s %6zu bytes\n", (size_t)((unsigned char*)h - heap), is_free(h) ? "FREE" : "used",
               block_size(h));
}

int main(void) {
    char* a = my_malloc(100);
    char* b = my_malloc(200);
    char* c = my_malloc(300);
    char* d = my_malloc(50);
    if (!a || !b || !c || !d) return 1;
    strcpy(a, "A survives");
    strcpy(d, "D survives");
    printf("alignment check: a%%16=%zu b%%16=%zu c%%16=%zu d%%16=%zu\n", (size_t)((uintptr_t)a % 16),
           (size_t)((uintptr_t)b % 16), (size_t)((uintptr_t)c % 16), (size_t)((uintptr_t)d % 16));
    heap_dump("after A(100) B(200) C(300) D(50):");

    my_free(b);
    heap_dump("after freeing B (a hole between A and C):");
    my_free(c); // merges backward with B
    heap_dump("after freeing C (coalesced with B):");

    char* e = my_malloc(450); // too big for B or C alone, fits in the merged hole
    printf("E(450) placed where B was? %s\n", e == b ? "yes" : "no");

    printf("data still intact: \"%s\", \"%s\"\n", a, d);
    my_free(a);
    my_free(e);
    my_free(d); // merges forward with the big tail AND backward with A+E
    heap_dump("after freeing everything (one big block again):");
    printf("my_malloc(70000) = %p (too big)\n", my_malloc(70000));
    return 0;
}
