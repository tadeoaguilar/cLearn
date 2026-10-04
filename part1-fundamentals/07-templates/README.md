# 07 — Templates & Generic Programming

> Goal: write code once and use it for many types, with full type safety and
> zero runtime overhead. Then constrain it with C++20 **concepts** so the
> error messages are readable.

## 1. Function templates

```cpp
template <typename T>
T max_of(T a, T b) { return a > b ? a : b; }

max_of(3, 7);          // T deduced as int → compiler generates max_of<int>
max_of(2.5, 1.0);      // T = double → a second, separate function
max_of<double>(3, 7.5);// explicit: avoids "conflicting types for T"
```

A template is **not** code. It's a *blueprint*. The compiler **instantiates**
a concrete function for each set of template arguments it sees. The result:
- **Zero overhead**: same speed as hand-written code. Compare with
  `virtual` (a runtime lookup) or Java/C# generics (type erasure/boxing).
- **Code bloat** is possible: one copy per type.
- **Templates live in headers**. The compiler needs the full definition at
  the point of use to instantiate it.

## 2. Class templates

```cpp
template <typename T, std::size_t N>
class FixedStack {
public:
    void push(const T& v) { data_[size_++] = v; }
    T pop() { return data_[--size_]; }
private:
    std::array<T, N> data_{};
    std::size_t size_ = 0;
};

FixedStack<int, 16> s;            // explicit args
std::vector v{1, 2, 3};           // CTAD (C++17): deduces std::vector<int>
```

Template parameters can be **types** (`typename T`), **values** (`std::size_t N`,
and since C++20 even some class types), or **templates** (rare).

## 3. Concepts (C++20): constraining templates

Without constraints, a bad type gives you pages of errors from deep inside the
template. Concepts state requirements **up front**:

```cpp
#include <concepts>

template <std::integral T>              // only integer types
T gcd(T a, T b);

template <typename T>
concept Shape = requires(const T& s) {  // define your own
    { s.area() } -> std::convertible_to<double>;
    { s.name() } -> std::convertible_to<std::string>;
};

void print_area(const Shape auto& s);   // abbreviated template syntax
```

Useful standard concepts: `std::integral`, `std::floating_point`,
`std::same_as<T,U>`, `std::convertible_to`, `std::derived_from`,
`std::equality_comparable`, `std::totally_ordered`, `std::invocable<F, Args...>`,
`std::ranges::range`, `std::copyable`, `std::movable`.

Four equivalent spellings:
```cpp
template <typename T> requires std::integral<T> T f(T);   // requires clause
template <std::integral T> T f(T);                        // constrained parameter
std::integral auto f(std::integral auto x);               // abbreviated
template <typename T> T f(T) requires std::integral<T>;   // trailing requires
```

## 4. Static polymorphism vs dynamic polymorphism

| | Templates/concepts | `virtual` |
|-|--------------------|-----------|
| Dispatch | compile time | runtime (vtable) |
| Speed | inlinable, fastest | indirect call |
| Heterogeneous containers | ❌ (need `std::variant`) | ✅ `vector<unique_ptr<Base>>` |
| Binary size | grows per type | one copy |
| Coupling | types just need the right *shape* (duck typing) | must inherit a base |

## 5. Specialization

```cpp
template <typename T> std::string type_name() { return "unknown"; }
template <> std::string type_name<int>() { return "int"; }      // full specialization

template <typename T> struct Box { ... };                        // primary
template <typename T> struct Box<T*> { ... };                     // partial (classes only)
```
In modern code, `if constexpr` and concepts usually replace specialization:

```cpp
template <typename T>
std::string describe(const T& v) {
    if constexpr (std::is_arithmetic_v<T>) return "number " + std::to_string(v);
    else if constexpr (std::is_same_v<T, std::string>) return "string " + v;
    else return "something else";
}  // discarded branches aren't even compiled for that T
```

## 6. Variadic templates & fold expressions

```cpp
template <typename... Args>                 // a parameter PACK of types
void log(const Args&... args) {             // a pack of parameters
    (std::cout << ... << args) << '\n';     // fold expression (C++17)
}
log("x=", 3, ", y=", 4.5);

template <typename... Ts> auto sum(Ts... xs) { return (xs + ... + 0); }
sizeof...(Ts);   // number of elements in the pack
```

## 7. Perfect forwarding (preview of ch. 9)

```cpp
template <typename T, typename... Args>
std::unique_ptr<T> make(Args&&... args) {           // Args&& here = "forwarding reference"
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...)); // preserve lvalue/rvalue-ness
}
```

## 8. Type traits (`<type_traits>`)

Compile-time questions about types: `std::is_integral_v<T>`,
`std::is_pointer_v<T>`, `std::remove_reference_t<T>`, `std::decay_t<T>`,
`std::conditional_t<cond, A, B>`, `std::is_same_v<A, B>`. They are the building
blocks behind concepts.

## 9. Templates and compile errors

Reading a template error: scroll to the **first** error, find the line in
**your** file, and look for `required from here` / `in instantiation of`.
With concepts, the error says plainly *"constraints not satisfied: T does not
satisfy std::integral"*.

## Examples
| File | Shows |
|------|-------|
| `examples/01_function_templates.cpp` | deduction, explicit args, multiple params, auto return |
| `examples/02_class_templates.cpp` | `FixedStack<T,N>`, `Pair<K,V>`, CTAD, member templates |
| `examples/03_concepts.cpp` | standard and custom concepts, overloading by concept |
| `examples/04_variadic_fold.cpp` | variadic `log`, `sum`, `all_same`, tuple printing |
| `examples/05_if_constexpr_traits.cpp` | `if constexpr`, type traits, specialization |
