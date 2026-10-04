# 21 — C Essentials for C++ Programmers

> Goal: write idiomatic C17. Know which C++ features are missing, what C
> uses instead, and the places where the *same syntax means something
> different*.

## 1. Same family, different language

C is (almost) a subset of C++ syntactically, but **not** semantically. A C
compiler is a different program with different rules:

```bash
cc  -std=c17   -Wall -Wextra -Wpedantic hello.c -o hello   # C
c++ -std=c++23 -Wall -Wextra -Wpedantic hello.cpp -o hello # C++
```

| C++ feature | What C does instead |
|-------------|---------------------|
| classes, methods | `struct` + free functions that take a `struct T*` as first parameter |
| constructors / destructors / RAII | `t_init()` / `t_destroy()` pairs that you call **yourself** |
| `private:` | **opaque types**: declare `struct T;` in the header, define it in the `.c` |
| `new` / `delete` | `malloc` / `free` (chapter 22) |
| references `T&` | pointers `T*` |
| overloading, default arguments | distinct names: `vec_push_int`, `vec_push_double` |
| templates | `void*` + size, or macros, or `_Generic` |
| exceptions | return codes, `errno`, out-parameters, `goto cleanup` |
| `std::string` | `char` arrays terminated by `'\0'` |
| `std::vector` | a struct `{ T* data; size_t len, cap; }` you write (ch. 22) |
| namespaces | name prefixes: `gpio_write`, `uart_init` |
| `bool`, `nullptr` | `<stdbool.h>` `bool`, `NULL` (C23 adds `bool` and `nullptr` as keywords) |
| `std::cout << x` | `printf("%d\n", x)`; the format **must** match the type |

## 2. Traps: same code, different meaning

```c
void f();          // C17: "f takes UNSPECIFIED arguments" (not checked!)
void f(void);      // C: "f takes no arguments". Always write (void) in C.

int* p = malloc(n * sizeof *p);   // C: void* converts implicitly. No cast needed.
                                  // C++: error without (int*). Don't cast in C: it hides a
                                  // missing #include <stdlib.h> on old compilers.

sizeof('a')        // C: sizeof(int) == 4.   C++: sizeof(char) == 1

struct Point { int x, y; };
struct Point p;    // C: the "struct" keyword is required...
typedef struct Point Point;   // ...unless you typedef it.
Point q;           // now this works

const int N = 10;
int arr[N];        // C: a variable-length array (VLA), not a constant! Use enum or #define.
enum { CAPACITY = 10 };   // the idiomatic C compile-time integer constant
```

## 3. Structs, initialization and compound literals

```c
typedef struct {
    float x, y;
} Vec2;

Vec2 a = {1.0f, 2.0f};
Vec2 b = {.y = 5.0f};               // designated initializer; x is zeroed
Vec2 c = {0};                       // zero everything, the universal C idiom
Vec2 sum = vec2_add(a, (Vec2){3, 4});   // compound literal: an unnamed temporary

int lookup[256] = {['a'] = 1, ['e'] = 1, ['i'] = 1};  // designated array elements
```

Structs are copied by value with `=`, passed by value, and returned by value,
just like a C++ aggregate. There is no `operator==`: compare field by field
(**never** `memcmp` structs, because padding bytes have unspecified values).

### Memory layout and padding

```c
struct Bad  { char a; double b; char c; };   // 24 bytes: 1 + 7 pad + 8 + 1 + 7 pad
struct Good { double b; char a; char c; };   // 16 bytes: 8 + 1 + 1 + 6 pad
```

Order members from largest to smallest. Inspect layouts with `sizeof`,
`offsetof` (`<stddef.h>`) and `_Alignof`. See `examples/02_structs_layout.c`.

## 4. Strings are just bytes

A C string is a `char` array whose end is marked by a `'\0'` byte. The
length is **not stored anywhere**: `strlen` walks until it finds the zero.

```
char s[8] = "hi";     ┌───┬───┬────┬────┬────┬────┬────┬────┐
                      │ h │ i │ \0 │ \0 │ \0 │ \0 │ \0 │ \0 │
                      └───┴───┴────┴────┴────┴────┴────┴────┘
const char* lit = "hi";  // points into read-only memory. Writing to it is UB.
```

| Avoid | Use | Why |
|-------|-----|-----|
| `gets` | `fgets(buf, sizeof buf, stdin)` | `gets` cannot know the buffer size (removed in C11) |
| `strcpy`, `strcat` | `snprintf(dst, size, "%s%s", a, b)` | bounds-checked, always terminates |
| `sprintf` | `snprintf` | same |
| `strncpy` | `snprintf` or `memcpy` + manual `'\0'` | `strncpy` does **not** terminate when truncating |
| `atoi` | `strtol` with an `end` pointer and an `errno` check | `atoi` cannot report errors |

`snprintf` returns the length it *would* have written. If that is
`>= size`, the output was truncated, and you can test for it.

## 5. The preprocessor

Runs before the compiler and does plain text substitution.

```c
#define BUF_SIZE 256                    // constant (prefer enum for ints)
#define MAX(a, b) ((a) > (b) ? (a) : (b))  // parenthesize EVERYTHING
                                        // MAX(i++, j) still evaluates i++ twice!
#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))  // only works on real arrays, not pointers

#define STR(x)  #x                      // stringify:   STR(foo) -> "foo"
#define CAT(a, b) a##b                  // token-paste: CAT(gpio_, init) -> gpio_init

#ifndef MYLIB_H                         // include guard (or #pragma once)
#define MYLIB_H
...
#endif
```

Multi-statement macros go in `do { ... } while (0)`, so they behave like one
statement after an `if`. **X-macros** generate several parallel tables from a
single list: see `examples/04_preprocessor.c`.

### `_Generic`: compile-time type dispatch (C11)

```c
#define print_val(x) _Generic((x), int: print_int, double: print_double, \
                                   const char*: print_str, char*: print_str)(x)
```

C's poor-man's overloading. `<tgmath.h>` is built with it.

## 6. Encapsulation with opaque types

```c
// stack.h: the public interface. Users can't see inside.
typedef struct Stack Stack;            // incomplete type
Stack* stack_create(size_t capacity);
void   stack_destroy(Stack* s);
bool   stack_push(Stack* s, int v);

// stack.c: the only file that knows the layout
struct Stack { int* items; size_t len, cap; };
```

Callers can hold a `Stack*` but cannot dereference it or create one on the
stack. This gives you `private:` plus ABI stability: you can add fields
without recompiling users. `FILE*` works this way.

## 7. Error handling without exceptions

Common conventions (pick one per library and **document** it):

- Return `0` on success, a negative error code on failure (POSIX, Linux kernel).
- Return a pointer, `NULL` on failure, with details in `errno`.
- Return `bool` and write the result through an out-parameter.

When a function acquires several resources, the cleanest pattern is
**`goto` cleanup**. It is C's stand-in for RAII, and the Linux kernel uses it
everywhere:

```c
int process(const char* path) {
    int rc = -1;
    FILE* f = fopen(path, "r");
    if (!f) goto out;
    char* buf = malloc(4096);
    if (!buf) goto close_file;
    if (fread(buf, 1, 4096, f) == 0) goto free_buf;
    rc = 0;                     // success
free_buf:   free(buf);
close_file: fclose(f);
out:        return rc;          // cleanup in reverse order of acquisition
}
```

## 8. Calling C from C++ (and back)

C++ **mangles** names to support overloading (`add(int,int)` becomes
`_Z3addii`). C does not (`add` stays `add`). A header shared by both
languages must turn mangling off:

```c
#ifdef __cplusplus
extern "C" {
#endif
int add(int a, int b);
#ifdef __cplusplus
}
#endif
```

That is how every C library (`libpq`, SQLite, OpenSSL) is usable from C++,
and how chapter 24's assembly functions are callable from C.

## Examples

| File | Shows |
|------|-------|
| `01_c_vs_cpp.c` | `(void)` parameters, `printf` formats, `bool`, `sizeof('a')`, `enum` constants |
| `02_structs_layout.c` | designated initializers, compound literals, padding, `offsetof` |
| `03_strings.c` | how strings are stored, safe copying with `snprintf`, `strtol` parsing, tokenizing |
| `04_preprocessor.c` | macros and their traps, stringify/paste, X-macros, `_Generic` |
| `05_function_pointers.c` | callbacks, `qsort`, dispatch tables: C's virtual functions |
| `06_errors_goto.c` | error codes, `errno`, the `goto cleanup` pattern |

Run them with `./scripts/check.sh --run 21-c-essentials`, or compile one by
hand: `cc -std=c17 -Wall -Wextra examples/03_strings.c && ./a.out`.

## Pitfalls

- Forgetting `(void)` in a declaration turns off argument checking.
- The `printf` format must match the type: `%zu` for `size_t`, `%ld` for `long`,
  `%p` for pointers (cast to `void*`), `%f` for both `float` and `double`.
  Compile with `-Wall` (`-Wformat`).
- `ARRAY_LEN(p)` on a pointer (for example an array parameter) silently returns `8/sizeof(T)`.
- Arrays decay to pointers when passed: `void f(int a[10])` is really `void f(int* a)`.
- Integer promotion: `uint8_t a = 200, b = 100; a + b` is the `int` 300, not 44.
- Signed overflow is undefined behaviour. Unsigned overflow wraps.

➡️ Next: [chapter 22](../22-c-memory/README.md) puts all of this to work managing memory.
