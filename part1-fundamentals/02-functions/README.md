# 02 — Functions

> Goal: write functions the idiomatic C++ way. That means choosing how to pass
> parameters, overloading, default arguments, recursion, and splitting code
> into headers and source files.

## 1. Declaring and defining

```cpp
int add(int a, int b);          // declaration (a.k.a. prototype): name + signature
int add(int a, int b) {         // definition: the body
    return a + b;
}
```

The compiler reads a file **top to bottom**. You can only call a function once
it has been *declared*. That's why headers exist: they hold declarations that
many `.cpp` files can `#include`.

## 2. Passing parameters: the most important decision

| Signature | Meaning | Use for |
|-----------|---------|---------|
| `void f(int x)` | **by value**: f gets its own copy | cheap types (int, double, small structs, `string_view`, `span`) |
| `void f(const std::string& s)` | **by const reference**: no copy, read-only | anything "big" you only read |
| `void f(std::string& s)` | **by reference**: f can modify the caller's object | output / in-out parameters |
| `void f(std::string* s)` | **by pointer**: like a reference, but can be `nullptr` | optional out-params, C APIs |
| `void f(std::string s)` then `std::move` | **by value + move** | "sink" params you'll store (ch. 9) |

```cpp
void shout(std::string s)        { s += "!"; }          // modifies a COPY
void shout_ref(std::string& s)   { s += "!"; }          // modifies the caller's string
void print(const std::string& s) { std::cout << s; }    // read-only, no copy
```

Rule of thumb: **return values instead of using out-parameters**. Modern
compilers elide copies of returned objects (RVO), so returning
`std::vector<int>` by value is cheap.

## 3. Return values

```cpp
double average(const std::vector<double>& v);   // single value
std::pair<int, int> min_max(...);                // two values
struct Stats { double mean, stddev; };           // several named values → struct
Stats compute(...);
auto [mean, sd] = compute(...);                  // structured binding (C++17)
std::optional<int> find_index(...);              // value or nothing (ch. 8)
```

Mark functions whose result must not be ignored with `[[nodiscard]]`.

## 4. Overloading

Several functions can share a name if their parameter lists differ:

```cpp
double area(double radius);              // circle
double area(double width, double height);// rectangle
```
The compiler picks the best match at compile time (overload resolution). The
return type alone **cannot** distinguish overloads.

## 5. Default arguments

```cpp
std::string greet(const std::string& name, const std::string& greeting = "Hello");
greet("Ada");               // "Hello, Ada"
greet("Ada", "Welcome");    // "Welcome, Ada"
```
Defaults go in the **declaration** (the header), and only on trailing parameters.

## 6. `inline`, `constexpr` and `consteval`

- `inline` lets a function be *defined* in a header that many `.cpp` files
  include, without violating the One Definition Rule. Despite the name, it is
  mostly not an optimization hint any more.
- `constexpr` means the function *may* run at compile time when its
  arguments are constants. It is implicitly `inline`.
- `consteval` (C++20) means the function *must* run at compile time.

## 7. Recursion

A function may call itself. You need a **base case**, and each step must get
closer to it. Deep recursion can overflow the stack (typically ~8 MB), so prefer
iteration for large inputs.

```cpp
unsigned long long factorial(unsigned n) { return n <= 1 ? 1 : n * factorial(n - 1); }
```

## 8. Function pointers and passing functions around

```cpp
int apply(int (*op)(int, int), int a, int b) { return op(a, b); }  // C-style
int apply(const std::function<int(int,int)>& op, int a, int b);     // any callable
template <typename F> int apply(F op, int a, int b);                // fastest (ch. 7)
apply([](int a, int b) { return a * b; }, 3, 4);                    // lambda (ch. 10)
```

## 9. Headers and source files

```
math_utils.hpp            math_utils.cpp                 main.cpp
──────────────            ──────────────                 ────────
#pragma once              #include "math_utils.hpp"      #include "math_utils.hpp"
int gcd(int, int);        int gcd(int a, int b) {...}    int main() { gcd(12, 18); }
```

- `#pragma once` (or classic `#ifndef X / #define X / #endif` *include guards*)
  stops a header from being pasted twice into one translation unit.
- Headers contain **declarations**, plus definitions of `inline`, `constexpr`
  and template functions. Sources (`.cpp`) contain the other **definitions**.
- Build both: `clang++ main.cpp math_utils.cpp -o app`. Each `.cpp` compiles
  separately, and the linker connects the call in `main.o` to the definition in
  `math_utils.o`.
- Forget to compile `math_utils.cpp` and you get a **linker error**:
  `Undefined symbols: gcd(int, int)`.

Chapter 12 builds a real multi-file project with CMake. In this chapter,
`examples/geometry.hpp` is a header-only module (all functions `inline`) used
by `examples/04_header_usage.cpp`.

## 10. Namespaces

```cpp
namespace geo {
    double area(double r);
}
geo::area(2.0);
using geo::area;         // bring one name in
// using namespace std;  // avoid in headers; it pollutes every includer
```

## 11. `static` and anonymous namespaces

A free function or variable marked `static`, or placed in an unnamed
`namespace { }`, is visible only inside its own `.cpp` file (*internal linkage*).
Use this for helpers that are not part of a module's public API.

## Pitfalls
| Pitfall | Fix |
|---------|-----|
| Returning a reference to a local variable | return by value |
| Passing `std::vector` by value accidentally | `const std::vector<T>&` |
| `using namespace std;` in a header | qualify names: `std::` |
| Non-`inline` function defined in a header included twice | add `inline`, or move it to a `.cpp` |
| Missing base case in recursion | stack overflow → crash |

## Examples
| File | Shows |
|------|-------|
| `examples/01_parameters.cpp` | value vs reference vs const reference vs pointer |
| `examples/02_overloading_defaults.cpp` | overloading, default args, `[[nodiscard]]` |
| `examples/03_recursion.cpp` | factorial, fibonacci (naive vs memoized), Hanoi |
| `examples/04_header_usage.cpp` + `geometry.hpp` | headers, namespaces, `inline`, `constexpr` |
| `examples/05_callbacks.cpp` | function pointers, `std::function`, lambdas as arguments |
