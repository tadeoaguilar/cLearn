# Modern C++ Cheatsheet

## Variables & types
```cpp
int i = 42;            auto x = 3.14;          // double
const int k = 10;      constexpr int N = 64;   // compile-time constant
std::string s = "hi";  std::string_view sv = s; // non-owning view
int a{5};              // brace init: no narrowing (int b{3.7} is an error)
using Id = std::int64_t;
```

## Control flow
```cpp
if (auto it = m.find(k); it != m.end()) { /* init-statement */ }
for (const auto& item : items) {}
switch (c) { case 'a': [[fallthrough]]; case 'b': break; default: break; }
```

## Functions
```cpp
int add(int a, int b = 0);                        // default arg
void read(const std::string& s);                  // pass big things by const&
void fill(std::vector<int>& out);                 // out-param by &
std::optional<User> find(Id id);                  // maybe-a-value
[[nodiscard]] bool save();                        // warn if result ignored
auto square = [](auto v) { return v * v; };       // generic lambda
```

## Classes
```cpp
class Point {
public:
    Point(double x, double y) : x_{x}, y_{y} {}   // member initializer list
    double x() const { return x_; }               // const member fn
    bool operator==(const Point&) const = default;
private:
    double x_{}, y_{};
};
struct Base { virtual ~Base() = default; virtual void f() = 0; };
struct Derived final : Base { void f() override {} };
```

## Ownership
| You want | Use |
|----------|-----|
| a value on the stack | `T t;` |
| sole owner of a heap object | `std::unique_ptr<T> p = std::make_unique<T>(...)` |
| shared ownership | `std::shared_ptr<T> p = std::make_shared<T>(...)` |
| observe without owning | `T&`, `const T&`, `T*`, `std::weak_ptr<T>` |
| a dynamic array | `std::vector<T>` |
| a view over contiguous data | `std::span<T>` / `std::string_view` |

## Containers
| Container | Use when |
|-----------|----------|
| `vector` | default choice; contiguous, fast iteration |
| `array<T,N>` | fixed size known at compile time |
| `deque` | push/pop at both ends |
| `unordered_map` | key → value, O(1) average lookup |
| `map` | key → value, **sorted**, O(log n) |
| `set` / `unordered_set` | unique values |

## Algorithms (`<algorithm>`, `<numeric>`, `<ranges>`)
```cpp
std::ranges::sort(v);
std::ranges::sort(people, {}, &Person::age);           // sort by projection
auto it = std::ranges::find(v, 42);
auto n  = std::ranges::count_if(v, [](int x){ return x % 2 == 0; });
auto sum = std::accumulate(v.begin(), v.end(), 0);
for (int x : v | std::views::filter(even) | std::views::transform(sq)) {}
std::erase_if(v, [](int x){ return x < 0; });          // C++20
```

## Errors
```cpp
try { throw std::runtime_error("boom"); }
catch (const std::exception& e) { std::cerr << e.what(); }
std::optional<int> parse(std::string_view);            // absence
std::expected<int, Error> parse(std::string_view);     // C++23: value or error
```

## Strings & formatting
```cpp
std::format("{} is {:.2f}", name, value);  // C++20
std::to_string(42); std::stoi("42");
s.starts_with("ab"); s.contains("x");      // C++20 / C++23
```

## Common compiler flags
`-std=c++23 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined`
