#include <stdio.h>
#include <string.h>

typedef int (*cmp_fn)(const void*, const void*);

static void generic_swap(void* a, void* b, size_t size) {
    unsigned char* pa = a;
    unsigned char* pb = b;
    for (size_t i = 0; i < size; i++) { // byte by byte: works for any type
        unsigned char t = pa[i];
        pa[i] = pb[i];
        pb[i] = t;
    }
}

static void generic_reverse(void* base, size_t n, size_t size) {
    if (n < 2) return;
    unsigned char* b = base;
    for (size_t lo = 0, hi = n - 1; lo < hi; lo++, hi--) generic_swap(b + lo * size, b + hi * size, size);
}

static void* generic_max(const void* base, size_t n, size_t size, cmp_fn cmp) {
    if (n == 0) return NULL;
    const unsigned char* b = base;
    const unsigned char* best = b;
    for (size_t i = 1; i < n; i++)
        if (cmp(b + i * size, best) > 0) best = b + i * size;
    return (void*)best; // cast away const, like the standard bsearch does
}

static void insertion_sort(void* base, size_t n, size_t size, cmp_fn cmp) {
    unsigned char* b = base;
    for (size_t i = 1; i < n; i++)
        for (size_t j = i; j > 0 && cmp(b + (j - 1) * size, b + j * size) > 0; j--)
            generic_swap(b + (j - 1) * size, b + j * size, size);
}

static int cmp_int(const void* a, const void* b) {
    int x = *(const int*)a, y = *(const int*)b;
    return (x > y) - (x < y);
}
static int cmp_double(const void* a, const void* b) {
    double x = *(const double*)a, y = *(const double*)b;
    return (x > y) - (x < y);
}
typedef struct {
    char name[16];
    int score;
} Player;
static int cmp_player_score(const void* a, const void* b) {
    return cmp_int(&((const Player*)a)->score, &((const Player*)b)->score);
}

int main(void) {
    int ints[] = {5, 2, 9, 1, 7};
    insertion_sort(ints, 5, sizeof ints[0], cmp_int);
    printf("sorted ints:");
    for (int i = 0; i < 5; i++) printf(" %d", ints[i]);
    generic_reverse(ints, 5, sizeof ints[0]);
    printf("\nreversed:   ");
    for (int i = 0; i < 5; i++) printf(" %d", ints[i]);

    double ds[] = {2.5, -1.0, 9.75, 3.0};
    printf("\nmax double: %.2f\n", *(double*)generic_max(ds, 4, sizeof ds[0], cmp_double));

    Player team[] = {{"ana", 30}, {"luis", 12}, {"carmen", 45}, {"beto", 27}};
    insertion_sort(team, 4, sizeof team[0], cmp_player_score);
    printf("players by score:");
    for (int i = 0; i < 4; i++) printf(" %s(%d)", team[i].name, team[i].score);
    Player* best = generic_max(team, 4, sizeof team[0], cmp_player_score);
    printf("\nbest player: %s\n", best->name);

    char word[] = "drawer";
    generic_reverse(word, strlen(word), 1);
    printf("generic_reverse(\"drawer\") = \"%s\"\n", word);
    return 0;
}
