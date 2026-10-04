# 05 — Memory Management & RAII

> Goal: understand where objects live, who is responsible for freeing them,
> and how modern C++ makes leaks and double-frees nearly impossible.

## 1. Storage durations

| Storage | Created | Destroyed | Example |
|---------|---------|-----------|---------|
| **Automatic** (stack) | when declared | at end of scope (`}`) | `int x; std::string s;` |
| **Dynamic** (heap / free store) | `new` / `make_unique` | `delete` / smart pointer dtor | `std::make_unique<T>()` |
| **Static** | before `main` (or first use) | after `main` | globals, `static` locals |
| **Thread** | thread start | thread end | `thread_local int x;` |

```
 ┌─────────── process memory ────────────┐
 │  code (text)                          │
 │  static data (globals)                │
 │  heap   ↓ grows   (new, malloc)       │  big, flexible, slower to allocate
 │                                       │
 │  stack  ↑ grows   (local variables)   │  small (~8MB), super fast, auto cleanup
 └───────────────────────────────────────┘
```

**Default to the stack.** Use the heap when:
- the object must outlive the current scope,
- its size is only known at runtime, or it's very large (containers do this for you),
- you need polymorphism through a base pointer.

Note that `std::vector<int> v(1'000'000);` puts a tiny handle on the stack. The
elements go on the heap, and `v`'s destructor frees them. You get heap storage
without manual management. **That pattern has a name: RAII.**

## 2. Raw `new` / `delete`, and why to avoid them

```cpp
Widget* w = new Widget{};   // allocate + construct
...
delete w;                   // destruct + free. Forget it → leak. Do it twice → crash.

int* arr = new int[100];
delete[] arr;               // arrays need delete[] (mismatch = UB)
```

Problems:
- **Leaks**: an early `return` or an exception skips the `delete`.
- **Double free**: two pointers "own" the same object.
- **Use after free**: dangling pointers.
- **Unclear ownership**: who deletes a `Widget*` returned from a function?

## 3. RAII: Resource Acquisition Is Initialization

> Tie every resource to the lifetime of an object. The constructor acquires
> it and the destructor releases it.

Destructors run **no matter how a scope exits**, whether by normal flow,
`return`, `break` or an exception. So cleanup is guaranteed.

```cpp
class File {
public:
    explicit File(const char* path) : f_{std::fopen(path, "r")} {
        if (!f_) throw std::runtime_error("open failed");
    }
    ~File() { std::fclose(f_); }
    File(const File&) = delete;            // a file handle shouldn't be copied
    File& operator=(const File&) = delete;
private:
    std::FILE* f_;
};
```

RAII is everywhere in the standard library: `std::vector` (memory),
`std::fstream` (files), `std::lock_guard` (mutexes), `std::unique_ptr` (any heap
object), `std::jthread` (threads). Unreal Engine has its own variants.

## 4. Smart pointers (`<memory>`)

### `std::unique_ptr<T>`: exclusive ownership
```cpp
auto w = std::make_unique<Widget>(args...);  // allocate
w->do_thing();
auto w2 = std::move(w);    // transfer ownership; w is now nullptr
// auto w3 = w2;           // ERROR: can't copy a unique owner
```                        // freed automatically when w2 goes out of scope
- Zero overhead compared with a raw pointer.
- **Your default choice** for heap objects.
- Custom deleters: `std::unique_ptr<FILE, decltype(&fclose)> f{fopen(...), &fclose};`

### `std::shared_ptr<T>`: shared ownership via reference counting
```cpp
auto a = std::make_shared<Texture>("hero.png"); // refcount = 1
auto b = a;                                     // refcount = 2
a.reset();                                      // refcount = 1
// object freed when the last shared_ptr dies
```
- Costs: a control block allocation and atomic increments and decrements.
- Use it only when ownership is genuinely shared (caches, graph nodes,
  async callbacks).

### `std::weak_ptr<T>`: a non-owning observer of a `shared_ptr`
Breaks **reference cycles**. Two `shared_ptr`s pointing at each other never
get freed. Check that the object still exists with `if (auto sp = weak.lock())`.

### Parameter conventions (C++ Core Guidelines)
| Parameter | Meaning |
|-----------|---------|
| `void f(Widget&)` / `const Widget&` | use the object; ownership irrelevant ← **most common** |
| `void f(Widget*)` | use it, may be null |
| `void f(std::unique_ptr<Widget>)` | f **takes** ownership (caller must `std::move`) |
| `void f(std::unique_ptr<Widget>&)` | f may reseat the caller's pointer (rare) |
| `void f(std::shared_ptr<Widget>)` | f **shares** ownership (keeps a copy) |

Don't pass smart pointers just to *use* the object. Pass `T&` or `T*`.

## 5. Rule of Zero / Three / Five

If your class manages a resource directly (a raw handle), the compiler-generated
copy and destroy operations are **wrong**: they copy the handle, not the
resource, which leads to a double free.

- **Rule of Three**: if you write a destructor, copy constructor or copy
  assignment, you probably need all three.
- **Rule of Five**: add the move constructor and move assignment (ch. 9).
- **Rule of Zero** (the goal): build classes from members that already manage
  themselves, and write **none** of the five.

```cpp
class Good {                    // Rule of Zero: everything correct for free
    std::string name_;
    std::vector<int> data_;
    std::unique_ptr<Impl> impl_;  // makes Good move-only, which is usually what you want
};
```

## 6. Copy-and-swap idiom

A simple, exception-safe way to write assignment for resource-owning classes:
```cpp
Buffer& operator=(Buffer other) noexcept {  // by value: copy (or move) happens here
    swap(*this, other);                      // swap guts
    return *this;                            // old guts destroyed with `other`
}
```

## 7. Common memory bugs and how to catch them

| Bug | Symptom | Catch with |
|-----|---------|------------|
| leak | memory grows | `-fsanitize=address` (LeakSanitizer on Linux), `leaks` on macOS, Valgrind |
| use-after-free | random crashes or corruption | ASan |
| double free | crash in `free` | ASan |
| buffer overflow | corruption | ASan, `.at()`, `std::span` |
| uninitialized read | random values | `-Wall`, MemorySanitizer, init everything |

```bash
clang++ -std=c++23 -g -fsanitize=address,undefined file.cpp && ./a.out
```

## Examples
| File | Shows |
|------|-------|
| `examples/01_stack_vs_heap.cpp` | lifetimes, raw new/delete, the leak on early return |
| `examples/02_raii_file.cpp` | an RAII file wrapper with exception safety |
| `examples/03_unique_ptr.cpp` | ownership transfer, factories, custom deleters, pimpl-ish |
| `examples/04_shared_weak_ptr.cpp` | refcounts, cycles, weak_ptr |
| `examples/05_rule_of_three_five.cpp` | a hand-written owning buffer done right |
