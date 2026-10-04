#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* ===================== stack.h: what users see ===================== */
typedef struct IntStack IntStack; // opaque: the layout is hidden
IntStack* stack_create(size_t capacity);
void stack_destroy(IntStack* s);
bool stack_push(IntStack* s, int value);
bool stack_pop(IntStack* s, int* out);
size_t stack_size(const IntStack* s);

/* ===================== stack.c: the implementation ===================== */
struct IntStack {
    size_t len, cap;
    int items[]; // flexible array member (C99): the array lives in the same allocation
};

IntStack* stack_create(size_t capacity) {
    IntStack* s = malloc(sizeof *s + capacity * sizeof s->items[0]);
    if (!s) return NULL;
    s->len = 0;
    s->cap = capacity;
    return s;
}

void stack_destroy(IntStack* s) { free(s); } // free(NULL) is a no-op, like delete nullptr

bool stack_push(IntStack* s, int value) {
    if (s->len == s->cap) return false;
    s->items[s->len++] = value;
    return true;
}

bool stack_pop(IntStack* s, int* out) {
    if (s->len == 0) return false;
    *out = s->items[--s->len];
    return true;
}

size_t stack_size(const IntStack* s) { return s->len; }

/* ===================== main.c: a user ===================== */
static char closing_for(char open) { return open == '(' ? ')' : open == '[' ? ']' : '}'; }

static bool balanced(const char* text) {
    IntStack* s = stack_create(64);
    if (!s) return false;
    bool ok = true;
    for (const char* p = text; *p && ok; p++) {
        if (*p == '(' || *p == '[' || *p == '{') {
            ok = stack_push(s, *p);
        } else if (*p == ')' || *p == ']' || *p == '}') {
            int open;
            ok = stack_pop(s, &open) && closing_for((char)open) == *p;
        }
    }
    ok = ok && stack_size(s) == 0;
    stack_destroy(s); // every create needs exactly one destroy
    return ok;
}

int main(void) {
    const char* tests[] = {"{[()()]}", "{[(])}", "((", "", "f(a[i], {b})"};
    for (int i = 0; i < 5; i++) printf("%-14s %s\n", tests[i], balanced(tests[i]) ? "balanced" : "NOT balanced");
    // IntStack local;   // error: variable has incomplete type. That's the point.
    return 0;
}
