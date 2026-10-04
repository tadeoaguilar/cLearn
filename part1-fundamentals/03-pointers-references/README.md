# 03 — Pointers, References, Arrays & Views

> Goal: understand memory addresses. That is the mental model behind C++
> performance and behind most of its bugs.

## 1. Memory model in one picture

```
 address   value          variable
 0x1000  ┌─────────┐
         │   42    │  ◄── int x = 42;
 0x1004  ├─────────┤
         │ 0x1000  │  ◄── int* p = &x;   (p holds x's ADDRESS)
 0x100C  └─────────┘
          int& r = x;  ◄── r is another NAME for x (no storage of its own, conceptually)
```

- `&x` gives you the **address of** x.
- `*p` **dereferences** p and gives you the object it points to.
- A **reference** `int& r = x;` is an alias. It must be bound when created and
  can never be re-bound.

## 2. Pointers

```cpp
int x = 10;
int* p = &x;       // p points to x
*p = 20;           // x is now 20
int* q = nullptr;  // points to nothing. Always initialize pointers!
if (q) { ... }     // nullptr converts to false
p = &y;            // pointers CAN be re-pointed
```

### `const` and pointers: read right-to-left
```cpp
const int* a;        // pointer to const int: can't modify *a, can re-point a
int* const b = &x;   // const pointer to int: can modify *b, can't re-point b
const int* const c = &x; // neither
```

## 3. References

```cpp
int x = 10;
int& r = x;        // r IS x
r = 30;            // x == 30
const int& cr = x; // read-only alias. Also binds to temporaries: const int& t = 5;
```

**When to use which?**
| Need | Use |
|------|-----|
| alias that always refers to a valid object | reference |
| "maybe no object" (optional) | pointer (`nullptr`) or `std::optional<std::reference_wrapper<T>>` |
| re-seatable handle | pointer |
| ownership | **neither**: use values, containers or smart pointers (ch. 5) |

In modern C++ a **raw pointer means "non-owning observer"**. If you see `new`
paired with a raw pointer, something is probably wrong (chapter 5).

## 4. Arrays and pointer arithmetic

```cpp
int arr[5] = {1, 2, 3, 4, 5};   // C array: fixed size, decays to int* easily
int* p = arr;                   // == &arr[0]
*(p + 2) == arr[2];             // pointer arithmetic moves by sizeof(int)
std::size(arr);                 // 5 (but NOT after decaying to a pointer!)
```

C arrays lose their size when passed to functions (*array-to-pointer decay*).
Prefer:
- `std::array<int, 5>`: fixed size, knows its size, can be copied like a value.
- `std::vector<int>`: dynamic size.
- `std::span<int>` (C++20): a non-owning **view** of any contiguous sequence.
  It is the right parameter type for "give me some ints".

```cpp
int sum(std::span<const int> values);   // accepts C arrays, std::array, std::vector
```

## 5. `std::string_view`

The string equivalent of `span`. It is a pointer plus a length that refers to
characters owned by *someone else*. Copying it is cheap, and it accepts string
literals and `std::string` without allocating.

```cpp
void log(std::string_view msg);
log("literal");          // no std::string allocation
log(some_std_string);    // fine
```
⚠️ A `string_view` must not outlive the string it views:
```cpp
std::string_view bad() { std::string s = "temp"; return s; } // DANGLING!
```

## 6. Lifetimes & dangling

The most common class of C++ bug is to **use something after it's gone**:

```cpp
int* dangling() { int local = 5; return &local; }  // local dies at return
int& also_bad() { int local = 5; return local; }

std::vector<int> v{1,2,3};
int& first = v[0];
v.push_back(4);       // may reallocate → `first` now dangles!
```

Tools: compile with `-fsanitize=address` (ASan) to catch these at runtime (see
ch. 12). Turn on `-Wall -Wextra`, which catches the obvious cases.

## 7. Pointers to pointers, `void*`, and C strings (briefly)

You'll meet these in C APIs: `char** argv`, `void* user_data`. `void*` means
"address of something of unknown type". You have to `static_cast` it back to
the right type. Modern C++ code rarely needs either.

```cpp
int main(int argc, char* argv[]) {   // command-line arguments
    for (int i = 0; i < argc; ++i) std::cout << argv[i] << '\n';
}
```

## Pitfalls
| Pitfall | Fix |
|---------|-----|
| Uninitialized pointer `int* p;` | `int* p = nullptr;` |
| Dereferencing `nullptr` | check before use, or use references |
| Returning address/reference of a local | return by value |
| Holding references into a `vector` across `push_back` | store indexes, or `reserve()` beforehand |
| `sizeof(arr)` inside a function taking `int arr[]` | use `std::span` |

## Examples
| File | Shows |
|------|-------|
| `examples/01_pointer_basics.cpp` | `&`, `*`, nullptr, const-pointer combos |
| `examples/02_references.cpp` | aliases, const refs binding to temporaries, references in loops |
| `examples/03_arrays_span.cpp` | C arrays, decay, `std::array`, `std::span`, pointer arithmetic |
| `examples/04_string_view.cpp` | cheap string parameters, tokenizing with views |
| `examples/05_dangling.cpp` | lifetime bugs explained (safe to run: the bugs are commented out) |
| `examples/06_command_line.cpp` | `argc`/`argv` |
