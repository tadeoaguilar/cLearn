// Struct initialization, copying, and memory layout.
#include <stdalign.h> // alignof
#include <stddef.h>   // offsetof
#include <stdio.h>

typedef struct {
    float x, y;
} Vec2;

static Vec2 vec2_add(Vec2 a, Vec2 b) { return (Vec2){a.x + b.x, a.y + b.y}; } // compound literal

struct Bad {   // members in "random" order
    char tag;
    double value;
    char flag;
};

struct Good {  // largest members first
    double value;
    char tag;
    char flag;
};

#define SHOW_FIELD(T, f) printf("  %-6s offset %2zu size %zu\n", #f, offsetof(T, f), sizeof(((T*)0)->f))

int main(void) {
    Vec2 a = {1.0f, 2.0f};
    Vec2 b = {.y = 5.0f}; // designated initializer, x = 0
    Vec2 zero = {0};      // zero-initialize everything
    Vec2 c = vec2_add(a, (Vec2){3, 4});
    printf("a=(%.1f,%.1f) b=(%.1f,%.1f) zero=(%.1f,%.1f) c=(%.1f,%.1f)\n", a.x, a.y, b.x, b.y, zero.x, zero.y, c.x,
           c.y);

    Vec2 copy = a; // structs copy by value
    copy.x = 99;
    printf("after copy.x = 99: a.x is still %.1f\n", a.x);

    // Designated array initializers: a lookup table for vowels.
    const char vowel[128] = {['a'] = 1, ['e'] = 1, ['i'] = 1, ['o'] = 1, ['u'] = 1};
    const char* word = "education";
    int count = 0;
    for (const char* p = word; *p; p++) count += vowel[(unsigned char)*p];
    printf("\"%s\" has %d vowels\n", word, count);

    printf("\nstruct Bad:  size %zu, align %zu\n", sizeof(struct Bad), alignof(struct Bad));
    SHOW_FIELD(struct Bad, tag);
    SHOW_FIELD(struct Bad, value);
    SHOW_FIELD(struct Bad, flag);
    printf("struct Good: size %zu, align %zu\n", sizeof(struct Good), alignof(struct Good));
    SHOW_FIELD(struct Good, value);
    SHOW_FIELD(struct Good, tag);
    SHOW_FIELD(struct Good, flag);
    printf("Reordering saved %zu bytes per struct; for 1M elements that's %zu KB\n",
           sizeof(struct Bad) - sizeof(struct Good), (sizeof(struct Bad) - sizeof(struct Good)) * 1000000 / 1024);
    return 0;
}
