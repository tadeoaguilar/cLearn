# 10 — The Modern C++ Toolkit (C++17/20/23)

> Goal: the everyday features that make modern C++ concise. Lambdas,
> structured bindings, `std::variant`, ranges and views, `constexpr`,
> `std::format`, `std::chrono`, and friends.

## 1. Lambdas in depth

```cpp
auto add = [](int a, int b) { return a + b; };
//          ^capture  ^params    ^body
```

| Capture | Meaning |
|---------|---------|
| `[]` | nothing |
| `[x]` | copy x (at creation time!) |
| `[&x]` | reference to x (careful: x must outlive the lambda) |
| `[=]` / `[&]` | everything used, by copy / by reference |
| `[this]` / `[*this]` | the current object by pointer / by copy |
| `[p = std::move(ptr)]` | init-capture: move a unique_ptr in, or compute a value |

```cpp
auto counter = [n = 0]() mutable { return ++n; };   // mutable: may modify its copies
auto generic = [](const auto& x) { return x.size(); }; // generic lambda (template)
auto typed = []<typename T>(const std::vector<T>& v) { return T{}; }; // C++20 template lambda
```

A lambda is syntactic sugar for an unnamed class with an `operator()`. The
captures become its data members. That's why each lambda has a unique type,
and why storing lambdas generically requires a template parameter or `std::function`.

⚠️ **Dangling captures**: returning a lambda that captured a local `[&]` from
a function is a classic bug. Capture by value instead.

## 2. Structured bindings

```cpp
auto [x, y] = std::pair{1, 2};
for (const auto& [key, value] : map) {}
auto [q, r] = std::div(17, 5);
struct Point { int x, y; }; auto [px, py] = Point{3, 4};
```

## 3. `std::variant`: a type-safe union

A `std::variant<A, B, C>` holds **exactly one** of A, B or C. It is the
modern alternative to class hierarchies when the set of types is closed.

```cpp
using Shape = std::variant<Circle, Rect, Triangle>;
Shape s = Circle{1.0};
std::holds_alternative<Circle>(s);
std::get<Circle>(s);              // throws std::bad_variant_access if wrong
std::get_if<Rect>(&s);            // pointer or nullptr

// std::visit + "overloaded" lambdas: exhaustive pattern matching
template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
double area = std::visit(overloaded{
    [](const Circle& c) { return 3.14 * c.r * c.r; },
    [](const Rect& r)   { return r.w * r.h; },
    [](const Triangle& t) { return 0.5 * t.b * t.h; },
}, s);   // forgetting a case is a COMPILE error
```

| | `virtual` hierarchy | `std::variant` |
|-|---------------------|----------------|
| Add a new type | easy (new class) | must update every visit |
| Add a new operation | must touch every class | easy (new visitor) |
| Memory | heap per object | inline, no allocation |

Game state machines and event and message types are great fits for variants.

## 4. `std::any` and `std::optional`

- `std::optional<T>`: maybe a T (ch. 8).
- `std::any`: any copyable type, checked at runtime with `std::any_cast`.
  It is rarely the right choice. Prefer variant.

## 5. Ranges & views (C++20)

Views are **lazy**, **composable**, and **non-owning** pipelines:

```cpp
using namespace std::views;
auto result = numbers
            | filter([](int n) { return n % 2 == 0; })
            | transform([](int n) { return n * n; })
            | take(5);
// nothing has been computed yet. Iterating `result` pulls values through the pipeline
```

Handy views: `iota(a, b)`, `filter`, `transform`, `take`, `drop`,
`take_while`, `drop_while`, `reverse`, `keys`, `values`, `split`, `join`,
`enumerate` (C++23), `zip` (C++23), `chunk` (C++23), `slide` (C++23).
`std::ranges::to<std::vector>()` (C++23) materializes a view into a container.

⚠️ A view refers to its source, so don't let the source die first.

## 6. Compile-time programming

```cpp
constexpr int fib(int n) { return n < 2 ? n : fib(n-1) + fib(n-2); }
constexpr int f10 = fib(10);        // computed by the compiler
consteval int must_be_ct(int x) { return x * 2; }  // C++20: compile-time only
constinit static int counter = 0;   // C++20: guaranteed static (not dynamic) init
static_assert(fib(10) == 55);
```
C++20 allows `std::vector` and `std::string` inside constexpr functions, as
long as they don't escape.

## 7. `std::format` and `std::print`

```cpp
std::format("{:>8.2f}|{:<6}|{:^5}|{:#x}|{:08b}", 3.14159, "ab", 'c', 255, 5);
std::println("Hello {}!", name);   // C++23 <print>
```
Custom types: specialize `std::formatter<T>` (see example 05).

## 8. `std::chrono`

```cpp
using namespace std::chrono_literals;
auto timeout = 250ms + 2s;
auto start = std::chrono::steady_clock::now();   // for measuring intervals
auto now = std::chrono::system_clock::now();     // wall clock time
std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};
```
Use `steady_clock` for timing and `system_clock` for timestamps. Durations
carry their units in the type, so you can't accidentally add seconds to
milliseconds without conversion.

## 9. Small but great

- `std::string_view`, `std::span` (ch. 3)
- `[[nodiscard]]`, `[[maybe_unused]]`, `[[likely]]`/`[[unlikely]]`, `[[fallthrough]]`
- Designated initializers: `Config c{.host = "localhost", .port = 8080};`
- `using enum Color;` inside a switch (C++20)
- `std::to_underlying(e)` (C++23)
- Three-way comparison `<=>` (ch. 4)
- `std::source_location` for logging file and line without macros

## Examples
| File | Shows |
|------|-------|
| `examples/01_lambdas.cpp` | captures, mutable, generic & template lambdas, IIFE, recursion |
| `examples/02_variant_visit.cpp` | shapes & a game-state machine with `std::variant` |
| `examples/03_ranges_views.cpp` | lazy pipelines, C++23 views, `ranges::to` |
| `examples/04_constexpr.cpp` | compile-time tables, `consteval`, `static_assert` |
| `examples/05_format_chrono.cpp` | `std::format`, custom formatter, `std::print`, chrono |
| `examples/06_misc_features.cpp` | designated initializers, `using enum`, `source_location`, attributes |
