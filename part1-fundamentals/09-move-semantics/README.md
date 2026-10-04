# 09 — Move Semantics & Perfect Forwarding

> Goal: understand how C++ avoids unnecessary copies, so you can write
> classes that are cheap to pass around and APIs that take ownership efficiently.

## 1. The problem

```cpp
std::vector<std::string> make_names();   // big result
std::vector<std::string> v = make_names();
```
Before C++11 this could deep-copy every string. Yet the temporary returned by
`make_names()` was about to be destroyed anyway. Why not **steal its buffer**?
That's a *move*.

## 2. Value categories (simplified)

- **lvalue**: has a name or identity, so you can take its address.
  `x`, `v[0]`, `*ptr`, a function returning `T&`.
- **rvalue**: a temporary, or something about to expire.
  `42`, `x + y`, `make_names()`, `std::move(x)`.

```cpp
void f(std::string& s);        // binds to lvalues only
void f(const std::string& s);  // binds to both (read-only)
void f(std::string&& s);       // binds to rvalues only → "you may steal from s"
```

## 3. Move constructor and move assignment

```cpp
class Buffer {
    std::size_t size_;
    int* data_;
public:
    Buffer(Buffer&& other) noexcept
        : size_{std::exchange(other.size_, 0)},
          data_{std::exchange(other.data_, nullptr)} {}   // steal + leave other empty
    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            size_ = std::exchange(other.size_, 0);
            data_ = std::exchange(other.data_, nullptr);
        }
        return *this;
    }
};
```
- A move is usually just a few pointer copies, O(1), no matter how much data there is.
- The moved-from object must stay **valid but unspecified**. It must be
  safe to destroy or assign to. Don't read its value afterwards.
- Mark moves **`noexcept`**, so containers will use them (ch. 8).
- With the **Rule of Zero** (members such as `std::vector`, `std::string`,
  `std::unique_ptr`), the compiler generates correct moves for free.

## 4. `std::move` doesn't move

`std::move(x)` is just a **cast to an rvalue reference**. It says "I'm done
with x, you may steal from it". The actual moving happens in the move
constructor or assignment that the cast selects.

```cpp
std::string a = "a long string that lives on the heap.....";
std::string b = std::move(a);   // b stole a's buffer; a is now (probably) ""
```

Don't `std::move`:
- a `const` object. The result silently copies, because `const T&&` can't bind to a move ctor.
- a local in a `return` statement: `return std::move(local);` *prevents* copy elision.
- something you use again later.

## 5. Copy elision & RVO

```cpp
Widget make() { Widget w; ...; return w; }   // NRVO: w is built directly in the caller's slot
Widget make2() { return Widget{}; }          // guaranteed elision since C++17
```
So **return by value freely**.

## 6. Passing parameters for "sink" functions

When a function stores its argument, take it **by value and move it in**:

```cpp
class Person {
    std::string name_;
public:
    explicit Person(std::string name) : name_{std::move(name)} {}  // one ctor handles both:
};
Person a{some_string};            // copy into param, then move into member
Person b{"literal"};              // construct param, then move
Person c{std::move(some_string)}; // move, then move. No copy at all.
```

## 7. Forwarding references & perfect forwarding

In a template, `T&&` with a **deduced** `T` is a *forwarding reference*
(sometimes called a "universal reference"). It binds to anything and remembers
whether the argument was an lvalue or an rvalue.

```cpp
template <typename T>
void wrapper(T&& arg) {
    target(std::forward<T>(arg));   // pass it on with the SAME value category
}
```
`std::forward<T>` casts back to an rvalue only if the original was an rvalue.
This is how `make_unique`, `emplace_back` and `std::thread` pass your arguments
through untouched.

Reference collapsing rules: `T& &` → `T&`, `T& &&` → `T&`, `T&& &` → `T&`,
`T&& &&` → `T&&`.

## 8. Move-only types

`std::unique_ptr`, `std::thread`, `std::fstream` and `std::jthread` can be
moved but not copied. That expresses unique ownership in the type system.
Store them in containers with `push_back(std::move(x))` or `emplace_back(...)`.

## Mental model summary

| You write | What happens |
|-----------|--------------|
| `T b = a;` | copy |
| `T b = std::move(a);` | move (a left empty-ish) |
| `T b = make_t();` | elided: neither copy nor move |
| `f(a)` where `f(T)` | copy into param |
| `f(std::move(a))` where `f(T)` | move into param |
| `f(a)` where `f(T&&)` | ❌ doesn't compile (lvalue) |

## Examples
| File | Shows |
|------|-------|
| `examples/01_value_categories.cpp` | lvalue/rvalue overloads, what `std::move` really is |
| `examples/02_move_ctor.cpp` | instrumented class: copies vs moves vs elision |
| `examples/03_sink_parameters.cpp` | by-value-and-move vs const& vs && overloads |
| `examples/04_perfect_forwarding.cpp` | forwarding references, `std::forward`, a `make` factory |
| `examples/05_move_benchmark.cpp` | timing copy vs move of large vectors |
