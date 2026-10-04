// A growable array of ints: the C version of std::vector<int>.
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int* data;
    size_t len;
    size_t cap;
} IntVec;

static void vec_init(IntVec* v) { *v = (IntVec){0}; }

static void vec_free(IntVec* v) {
    free(v->data);
    *v = (IntVec){0};
}

static bool vec_reserve(IntVec* v, size_t want) {
    if (want <= v->cap) return true;
    size_t new_cap = v->cap ? v->cap : 4;
    while (new_cap < want) new_cap *= 2; // geometric growth → amortized O(1) push
    int* tmp = realloc(v->data, new_cap * sizeof *tmp);
    if (!tmp) return false;
    printf("    [realloc cap %zu → %zu]\n", v->cap, new_cap);
    v->data = tmp;
    v->cap = new_cap;
    return true;
}

static bool vec_push(IntVec* v, int value) {
    if (!vec_reserve(v, v->len + 1)) return false;
    v->data[v->len++] = value;
    return true;
}

static bool vec_pop(IntVec* v, int* out) {
    if (v->len == 0) return false;
    *out = v->data[--v->len];
    return true;
}

static bool vec_insert(IntVec* v, size_t index, int value) {
    if (index > v->len || !vec_reserve(v, v->len + 1)) return false;
    // Shift the tail right by one. Source and destination overlap, so this needs
    // memmove, never memcpy.
    memmove(&v->data[index + 1], &v->data[index], (v->len - index) * sizeof *v->data);
    v->data[index] = value;
    v->len++;
    return true;
}

static void vec_shrink_to_fit(IntVec* v) {
    if (v->len == v->cap) return;
    if (v->len == 0) {
        vec_free(v);
        return;
    }
    int* tmp = realloc(v->data, v->len * sizeof *tmp);
    if (tmp) { // if shrinking fails, keeping the larger block is fine
        v->data = tmp;
        v->cap = v->len;
    }
}

static void vec_print(const IntVec* v) {
    printf("len=%zu cap=%zu [", v->len, v->cap);
    for (size_t i = 0; i < v->len; i++) printf(i ? ", %d" : "%d", v->data[i]);
    printf("]\n");
}

int main(void) {
    IntVec v;
    vec_init(&v);
    printf("pushing 0..19 (watch how rarely realloc runs):\n");
    for (int i = 0; i < 20; i++)
        if (!vec_push(&v, i * i)) goto oom;
    vec_print(&v);

    int last;
    while (v.len > 5) vec_pop(&v, &last);
    if (!vec_insert(&v, 2, -1)) goto oom;
    printf("after popping down to 5 and inserting -1 at index 2:\n");
    vec_print(&v);
    vec_shrink_to_fit(&v);
    printf("after shrink_to_fit: ");
    vec_print(&v);

    vec_free(&v);
    return 0;
oom:
    fprintf(stderr, "out of memory\n");
    vec_free(&v);
    return 1;
}
