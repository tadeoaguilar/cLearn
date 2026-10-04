# 01 — C++ Basics

> Goal: understand how a C++ program gets built and runs, and the core building
> blocks: types, variables, operators, control flow and console I/O.

## 1. From source code to executable

Python and JavaScript are interpreted, or JIT-compiled while they run. C++ is
compiled **ahead of time** to native machine code:

```
 hello.cpp ──► [preprocessor] ──► [compiler] ──► hello.o ──► [linker] ──► hello (executable)
               #include, #define    C++ → asm/obj               + libraries (libc++)
```

1. **Preprocessor**: handles lines starting with `#`. `#include <iostream>` literally
   pastes the contents of the header into your file.
2. **Compiler**: turns each `.cpp` (a *translation unit*) into an object file.
   Syntax and type errors are reported here.
3. **Linker**: stitches object files and libraries together. Errors such as
   `undefined symbol` come from this stage.

Consequences:
- Type errors are caught **before** the program runs.
- There is no runtime "VM". You get a native binary for one OS and CPU.
- You must recompile after every change.

## 2. Anatomy of a program

```cpp
#include <iostream>   // bring in the I/O library declarations

int main() {           // the entry point; returns an exit code to the OS
    std::cout << "Hello, C++!\n";   // std:: = the standard library namespace
    return 0;          // 0 = success (can be omitted in main only)
}
```

- `std::cout` is the standard output stream. `<<` "pushes" values into it.
- Statements end with `;`. Blocks are delimited by `{}`.
- Prefer `'\n'` over `std::endl`. `endl` also *flushes* the stream, which is slow.

## 3. Fundamental types

| Type | Typical size | Example | Notes |
|------|--------------|---------|-------|
| `bool` | 1 byte | `true` | |
| `char` | 1 byte | `'A'` | a small integer; signedness is implementation-defined |
| `int` | 4 bytes | `42` | at least 16 bits by the standard; 32 in practice |
| `long long` | 8 bytes | `42LL` | |
| `std::int32_t`, `std::uint64_t` | exact | | from `<cstdint>`; use when size matters |
| `std::size_t` | 8 bytes (64-bit) | `v.size()` | unsigned; sizes & indexes |
| `float` | 4 bytes | `3.14f` | ~7 decimal digits |
| `double` | 8 bytes | `3.14` | ~15 digits; **default for real numbers** |
| `std::string` | — | `"text"` | from `<string>`; owns its characters |

> ⚠️ Sizes are **platform-dependent** except for the `<cstdint>` fixed-width types.
> Use `sizeof(T)` to check.

### Signed vs unsigned pitfalls
```cpp
unsigned int u = 0;
u = u - 1;               // wraps around to 4294967295 — well-defined but surprising
int i = INT_MAX; i + 1;  // signed overflow is UNDEFINED BEHAVIOR
std::vector<int> v;
for (int k = 0; k < v.size() - 1; ++k)  // v.size()-1 underflows when v is empty!
```

## 4. Variables and initialization

```cpp
int a = 5;        // copy initialization
int b(5);         // direct initialization
int c{5};         // brace (uniform) initialization — PREFER THIS
int d{};          // value-initialized → 0
int e;            // UNINITIALIZED: reading it is undefined behavior!
auto f = 5.0;     // type deduced: double
const int g = 7;  // cannot change
constexpr int h = 7 * 6;  // evaluated at compile time
```

Brace initialization **forbids narrowing**: `int x{3.7};` fails to compile, while
`int x = 3.7;` silently truncates to 3.

### `auto`
`auto` asks the compiler to deduce the type from the initializer. It's still
static typing. The type is fixed at compile time. Use it when the type is obvious
or very long (`auto it = map.begin();`).

## 5. Operators

- Arithmetic: `+ - * / %`. **Integer division truncates**: `7 / 2 == 3`, `7 / 2.0 == 3.5`.
- Comparison: `== != < > <= >=`, and C++20's three-way `<=>`.
- Logical: `&& || !`, which short-circuit.
- Increment: `++i` (prefer) vs `i++`.
- Compound: `+= -= *= /=`.
- Casting: `static_cast<double>(x)`. **Avoid C-style casts** like `(double)x`.

## 6. Control flow

```cpp
if (x > 0) { ... } else if (x < 0) { ... } else { ... }

switch (option) {          // works on integers, chars and enums
    case 1:  run();  break;
    case 2:  stop(); break; // forgetting `break` falls through!
    default: help();
}

for (int i = 0; i < 10; ++i) { ... }  // classic loop
for (const auto& x : container) { ... } // range-based loop — prefer this
while (cond) { ... }
do { ... } while (cond);            // runs at least once
```

C++17 lets `if` and `switch` have an *init-statement*:
```cpp
if (int n = compute(); n > 10) { /* n is scoped to the if/else */ }
```

## 7. Console input

```cpp
int age;
if (std::cin >> age) { /* parsed OK */ } else { /* bad input or EOF */ }

std::string line;
std::getline(std::cin, line);   // whole line, including spaces
```
`std::cin >> x` stops at whitespace. Mixing it with `getline` needs care,
because a leftover `'\n'` stays in the buffer. Call `std::cin.ignore()` first.

## 8. Strings

```cpp
#include <string>
std::string name = "Ada";
name += " Lovelace";
name.size();              // 12
name[0];                  // 'A'
name.substr(0, 3);        // "Ada"
name.find("Love");        // index or std::string::npos
std::to_string(3.5);      // "3.500000"
std::stoi("42");          // 42 (throws on bad input)
```

String literals like `"hi"` are *not* `std::string`. They are `const char[3]`
arrays, a C heritage. Use `std::string` (owns memory) or `std::string_view`
(cheap non-owning view, see chapter 3).

## 9. Enumerations

```cpp
enum class Color { Red, Green, Blue };   // scoped: Color::Red, no implicit int conversion
Color c = Color::Green;
```
Prefer `enum class` over plain `enum`. Plain enums leak their names into the
surrounding scope and convert to `int` silently.

## 10. Formatting output (C++20)

```cpp
#include <format>
std::cout << std::format("{} has {} items costing ${:.2f}\n", name, n, price);
```

## Common beginner pitfalls
| Pitfall | Fix |
|---------|-----|
| Using an uninitialized variable | always initialize: `int x{};` |
| `if (x = 5)` (assignment!) | enable `-Wall`; write `if (x == 5)` |
| Integer division surprise | cast one operand: `static_cast<double>(a) / b` |
| Comparing signed with unsigned | use `std::ssize(v)` or `std::size_t` indexes |
| Forgetting `break` in `switch` | use `[[fallthrough]]` when intentional |

## Examples in this chapter
| File | Shows |
|------|-------|
| `examples/01_hello_world.cpp` | minimal program |
| `examples/02_types_and_sizes.cpp` | `sizeof`, limits, fixed-width ints |
| `examples/03_variables_init.cpp` | initialization forms, `auto`, `const`, `constexpr` |
| `examples/04_control_flow.cpp` | if/switch/loops, init-statements |
| `examples/05_strings_and_io.cpp` | strings, `std::format`, safe input parsing |
| `examples/06_enums.cpp` | `enum class` + switch |

Next: [exercises](exercises/README.md)
