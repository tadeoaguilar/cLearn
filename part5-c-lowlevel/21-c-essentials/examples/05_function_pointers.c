// Function pointers: callbacks, qsort and hand-made "virtual functions".
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char* name;
    int age;
} Person;

// qsort comparators receive const void* and must cast to the real type.
static int by_age(const void* a, const void* b) {
    const Person* pa = a;
    const Person* pb = b;
    return (pa->age > pb->age) - (pa->age < pb->age); // avoids the overflow of `a - b`
}
static int by_name(const void* a, const void* b) {
    return strcmp(((const Person*)a)->name, ((const Person*)b)->name);
}

// A callback with a "user data" pointer, the C replacement for a capturing lambda.
typedef void (*visit_fn)(int value, void* user);
static void for_each(const int* arr, size_t n, visit_fn fn, void* user) {
    for (size_t i = 0; i < n; i++) fn(arr[i], user);
}
static void accumulate(int v, void* user) { *(long*)user += v; }

// "Polymorphism": a struct of function pointers (a vtable) + a data pointer.
typedef struct Shape Shape;
typedef struct {
    double (*area)(const Shape*);
    const char* (*name)(void);
} ShapeVTable;
struct Shape {
    const ShapeVTable* vt;
    double a, b; // circle uses a = radius; rect uses a, b
};
static double circle_area(const Shape* s) { return 3.14159265 * s->a * s->a; }
static const char* circle_name(void) { return "circle"; }
static double rect_area(const Shape* s) { return s->a * s->b; }
static const char* rect_name(void) { return "rect"; }
static const ShapeVTable CIRCLE = {circle_area, circle_name};
static const ShapeVTable RECT = {rect_area, rect_name};

// A dispatch table: look up a function by name instead of writing a long if/else chain.
static int op_add(int a, int b) { return a + b; }
static int op_sub(int a, int b) { return a - b; }
static int op_mul(int a, int b) { return a * b; }
typedef struct {
    const char* name;
    int (*fn)(int, int);
} Command;
static const Command COMMANDS[] = {{"add", op_add}, {"sub", op_sub}, {"mul", op_mul}};

int main(void) {
    Person people[] = {{"Carmen", 35}, {"Ana", 28}, {"Luis", 41}, {"Beto", 28}};
    size_t n = sizeof people / sizeof people[0];

    qsort(people, n, sizeof people[0], by_age);
    printf("by age: ");
    for (size_t i = 0; i < n; i++) printf("%s(%d) ", people[i].name, people[i].age);
    qsort(people, n, sizeof people[0], by_name);
    printf("\nby name: ");
    for (size_t i = 0; i < n; i++) printf("%s ", people[i].name);
    printf("\n");

    int values[] = {1, 2, 3, 4, 5};
    long total = 0;
    for_each(values, 5, accumulate, &total);
    printf("sum via callback = %ld\n", total);

    Shape shapes[] = {{&CIRCLE, 1.0, 0}, {&RECT, 3.0, 4.0}};
    for (size_t i = 0; i < 2; i++) printf("%s area = %.2f\n", shapes[i].vt->name(), shapes[i].vt->area(&shapes[i]));


    const char* script[] = {"mul", "sub", "pow"};
    for (size_t i = 0; i < 3; i++) {
        const Command* cmd = NULL;
        for (size_t j = 0; j < sizeof COMMANDS / sizeof COMMANDS[0]; j++)
            if (strcmp(COMMANDS[j].name, script[i]) == 0) cmd = &COMMANDS[j];
        if (cmd) printf("%s(6, 7) = %d\n", cmd->name, cmd->fn(6, 7));
        else printf("%s: unknown command\n", script[i]);
    }
    return 0;
}
