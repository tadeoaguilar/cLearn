// Macros: useful, dangerous, and occasionally magical.
#include <stdio.h>

#define SQUARE_BAD(x) x * x
#define SQUARE(x) ((x) * (x))
#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))
#define STR(x) #x
#define XSTR(x) STR(x) // expand x first, then stringify
#define VERSION_MAJOR 2

// Multi-statement macro: do { } while (0) makes it act like one statement.
#define SWAP_INT(a, b)                                                                                                 \
    do {                                                                                                               \
        int tmp_ = (a);                                                                                                \
        (a) = (b);                                                                                                     \
        (b) = tmp_;                                                                                                    \
    } while (0)

// X-macro: one list generates an enum AND its matching names table, which
// can never get out of sync.
#define COLOR_LIST(X)                                                                                                  \
    X(RED, 0xFF0000)                                                                                                   \
    X(GREEN, 0x00FF00)                                                                                                 \
    X(BLUE, 0x0000FF)

#define AS_ENUM(name, rgb) COLOR_##name,
#define AS_NAME(name, rgb) #name,
#define AS_RGB(name, rgb) rgb,
typedef enum { COLOR_LIST(AS_ENUM) COLOR_COUNT } Color;
static const char* const color_names[] = {COLOR_LIST(AS_NAME)};
static const unsigned color_rgb[] = {COLOR_LIST(AS_RGB)};

// _Generic: choose a function based on the static type of the argument.
static void print_int(int v) { printf("int %d\n", v); }
static void print_double(double v) { printf("double %g\n", v); }
static void print_str(const char* v) { printf("string \"%s\"\n", v); }
#define print_val(x) _Generic((x), int: print_int, double: print_double, char*: print_str, const char*: print_str)(x)

int main(void) {
    printf("SQUARE_BAD(1 + 2) = %d   (expands to 1 + 2 * 1 + 2)\n", SQUARE_BAD(1 + 2));
    printf("SQUARE(1 + 2)     = %d\n", SQUARE(1 + 2));

    // SQUARE(i++) would expand to ((i++) * (i++)): two unsequenced modifications,
    // which is undefined behaviour (clang -Wall even warns). A function has no such trap:
    // arguments are evaluated exactly once. Prefer static inline functions to macros.

    int nums[] = {4, 8, 15, 16, 23, 42};
    printf("ARRAY_LEN(nums) = %zu\n", ARRAY_LEN(nums));

    int a = 1, b = 2;
    if (a < b) SWAP_INT(a, b); // works after an unbraced if, thanks to do/while(0)
    else printf("never\n");
    printf("after swap: a=%d b=%d\n", a, b);

    printf("STR(VERSION_MAJOR) = %s, XSTR(VERSION_MAJOR) = %s\n", STR(VERSION_MAJOR), XSTR(VERSION_MAJOR));
    printf("compiled from %s line %d on %s\n", __FILE__, __LINE__, __DATE__);

    for (int c = 0; c < COLOR_COUNT; c++) printf("%-5s = #%06X\n", color_names[c], color_rgb[c]);

    print_val(42);
    print_val(3.14);
    print_val("hello");

#if defined(__clang__)
    printf("compiler: clang %s\n", __clang_version__);
#elif defined(__GNUC__)
    printf("compiler: gcc %d.%d\n", __GNUC__, __GNUC_MINOR__);
#endif
    return 0;
}
