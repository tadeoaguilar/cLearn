// A pool allocator: O(1) alloc/free of fixed-size blocks via an intrusive free list.
#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// A free block stores the "next" pointer inside itself, so the free list needs no extra memory.
typedef union FreeNode {
    union FreeNode* next;
    max_align_t align_; // makes every block suitably aligned for any type
} FreeNode;

typedef struct {
    unsigned char* memory;
    size_t block_size;
    size_t block_count;
    size_t in_use;
    FreeNode* free_list;
} Pool;

static int pool_init(Pool* p, size_t object_size, size_t count) {
    // A block must hold at least the free-list pointer and keep alignment for the next block.
    size_t bs = object_size < sizeof(FreeNode) ? sizeof(FreeNode) : object_size;
    bs = (bs + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1);
    p->memory = malloc(bs * count);
    if (!p->memory) return -1;
    p->block_size = bs;
    p->block_count = count;
    p->in_use = 0;
    p->free_list = NULL;
    // Thread every block onto the free list (in reverse, so block 0 is handed out first).
    for (size_t i = count; i-- > 0;) {
        FreeNode* node = (FreeNode*)(p->memory + i * bs);
        node->next = p->free_list;
        p->free_list = node;
    }
    return 0;
}

static void pool_destroy(Pool* p) {
    free(p->memory);
    *p = (Pool){0};
}

static void* pool_alloc(Pool* p) {
    FreeNode* node = p->free_list;
    if (!node) return NULL; // exhausted
    p->free_list = node->next;
    p->in_use++;
    return node;
}

static void pool_free(Pool* p, void* ptr) {
    if (!ptr) return;
    FreeNode* node = ptr;
    node->next = p->free_list; // push onto the free list
    p->free_list = node;
    p->in_use--;
}

static size_t pool_index(const Pool* p, const void* ptr) {
    return (size_t)((const unsigned char*)ptr - p->memory) / p->block_size;
}

typedef struct {
    int id;
    double x, y;
    int target_id;
} Enemy;

int main(void) {
    Pool pool;
    if (pool_init(&pool, sizeof(Enemy), 5) != 0) return 1;
    printf("sizeof(Enemy) = %zu, block size = %zu, capacity = %zu\n", sizeof(Enemy), pool.block_size,
           pool.block_count);

    Enemy* e[6] = {0};
    for (int i = 0; i < 6; i++) {
        e[i] = pool_alloc(&pool);
        if (e[i]) {
            *e[i] = (Enemy){.id = i};
            printf("alloc enemy %d → block %zu\n", i, pool_index(&pool, e[i]));
        } else {
            printf("alloc enemy %d → pool exhausted (NULL)\n", i);
        }
    }

    printf("free enemies 1 and 3\n");
    pool_free(&pool, e[1]);
    pool_free(&pool, e[3]);

    Enemy* reused = pool_alloc(&pool);
    printf("new alloc reuses block %zu (LIFO: the most recently freed block, still warm in cache)\n",
           pool_index(&pool, reused));
    printf("in use: %zu / %zu\n", pool.in_use, pool.block_count);

    pool_destroy(&pool); // releases every block, live or not
    return 0;
}
