# 21 — Exercises: C Essentials

Compile with `cc -std=c17 -Wall -Wextra -Wpedantic`. Fix every warning.

### Ex 1 — Your own string library ⭐
Implement these without `<string.h>`:
- `size_t my_strlen(const char* s)`
- `int my_strcmp(const char* a, const char* b)` returning <0, 0 or >0, like `strcmp`
- `size_t my_strlcpy(char* dst, const char* src, size_t size)`: copies at most
  `size - 1` chars, **always** NUL-terminates (if `size > 0`), and returns `strlen(src)`
  so the caller can detect truncation (BSD's `strlcpy`)
- `void my_strrev(char* s)`: reverses in place

Test each one against the real `<string.h>` function in `main`.
→ `solutions/ex01_my_string.c`

### Ex 2 — A CSV splitter that keeps empty fields ⭐⭐
`strtok` treats `"a,,b"` as two fields. Write
`size_t split(char* line, char sep, char* fields[], size_t max_fields)`
that modifies `line` in place (replaces separators with `'\0'`) and stores a
pointer to each field, keeping empty fields. It returns the number of fields.
Then parse `"Ana,28,,Mexico City"` into a `Person { char name[32]; int age; char city[32]; }`
with `snprintf` and `strtol`, reporting missing or invalid fields.
→ `solutions/ex02_csv_split.c`

### Ex 3 — An opaque stack ⭐⭐
Design an `IntStack` with an **opaque type**: users see only
`typedef struct IntStack IntStack;` and functions `stack_create(capacity)`,
`stack_destroy`, `stack_push` (returns `bool`, false when full),
`stack_pop(IntStack*, int* out)` (returns `bool`), `stack_size`.
Put the "header" part at the top of the file and the "implementation" below it.
Use it to check whether the brackets in `"{[()()]}"` and `"{[(])}"` are balanced.
(Keep it single-file: in a real project these would be `stack.h` and `stack.c`.)
→ `solutions/ex03_opaque_stack.c`

### Ex 4 — Generic algorithms with `void*` ⭐⭐⭐
Write C's answer to templates:
- `void generic_swap(void* a, void* b, size_t size)`
- `void generic_reverse(void* base, size_t n, size_t size)`
- `void* generic_max(const void* base, size_t n, size_t size, int (*cmp)(const void*, const void*))`
- `void insertion_sort(void* base, size_t n, size_t size, int (*cmp)(const void*, const void*))`

Test them on an `int` array, a `double` array, and an array of `struct { char name[16]; int score; }`.
Hint: byte-wise pointer arithmetic needs `char*` (or `unsigned char*`), because
arithmetic on `void*` is a GNU extension.
→ `solutions/ex04_generic_void.c`
