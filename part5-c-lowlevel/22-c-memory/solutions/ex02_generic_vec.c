#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// "Templates" in C: a macro that stamps out a typed struct and its functions.
#define DEFINE_VEC(T, Name)                                                                                            \
    typedef struct {                                                                                                   \
        T* data;                                                                                                       \
        size_t len, cap;                                                                                               \
    } Name;                                                                                                            \
                                                                                                                       \
    static inline bool Name##_push(Name* v, T value) {                                                                 \
        if (v->len == v->cap) {                                                                                        \
            size_t cap = v->cap ? v->cap * 2 : 4;                                                                      \
            T* tmp = realloc(v->data, cap * sizeof *tmp);                                                              \
            if (!tmp) return false;                                                                                    \
            v->data = tmp;                                                                                             \
            v->cap = cap;                                                                                              \
        }                                                                                                              \
        v->data[v->len++] = value;                                                                                     \
        return true;                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    static inline T* Name##_get(Name* v, size_t i) { return i < v->len ? &v->data[i] : NULL; }                         \
                                                                                                                       \
    static inline void Name##_free(Name* v) {                                                                          \
        free(v->data);                                                                                                 \
        v->data = NULL;                                                                                                \
        v->len = v->cap = 0;                                                                                           \
    }

typedef struct {
    int x, y;
} Point;

DEFINE_VEC(int, IntVec)
DEFINE_VEC(double, DoubleVec)
DEFINE_VEC(Point, PointVec)

int main(void) {
    IntVec ints = {0};
    DoubleVec ds = {0};
    PointVec pts = {0};
    bool ok = true;

    for (int i = 0; i < 10 && ok; i++) ok = IntVec_push(&ints, i * 10);
    for (int i = 0; i < 5 && ok; i++) ok = DoubleVec_push(&ds, i / 4.0);
    for (int i = 0; i < 3 && ok; i++) ok = PointVec_push(&pts, (Point){i, -i});

    if (ok) {
        printf("ints[7] = %d (len %zu, cap %zu)\n", *IntVec_get(&ints, 7), ints.len, ints.cap);
        printf("doubles:");
        for (size_t i = 0; i < ds.len; i++) printf(" %.2f", *DoubleVec_get(&ds, i));
        printf("\npoints:");
        for (size_t i = 0; i < pts.len; i++) {
            const Point* pt = PointVec_get(&pts, i);
            printf(" (%d,%d)", pt->x, pt->y);
        }
        printf("\nout of range get → %p\n", (void*)IntVec_get(&ints, 99));
        // IntVec_push(&ints, (Point){0, 0});   // compile error: the type is checked, unlike void*
    }

    IntVec_free(&ints);
    DoubleVec_free(&ds);
    PointVec_free(&pts);
    return ok ? 0 : 1;
}
