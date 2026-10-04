# 04 — Classes & Object-Oriented Programming

> Goal: model data and behavior with classes. Covers encapsulation,
> constructors and destructors, const-correctness, operator overloading,
> inheritance, and runtime polymorphism.

## 1. `struct` vs `class`

They are the same thing, except for the default access level:
`struct` members are `public` by default and `class` members are `private`.
Convention: use `struct` for plain data bags, and `class` when you have
**invariants** to protect.

```cpp
struct Point { double x{}; double y{}; };   // aggregate: Point p{1, 2};

class BankAccount {
public:
    explicit BankAccount(std::string owner, double initial = 0.0);
    void deposit(double amount);       // modifies → non-const
    double balance() const;            // read-only → const
private:
    std::string owner_;
    double balance_{0.0};              // invariant: never negative
};
```

## 2. Constructors

```cpp
BankAccount::BankAccount(std::string owner, double initial)
    : owner_{std::move(owner)}, balance_{initial}   // member initializer list
{
    if (initial < 0) throw std::invalid_argument("negative initial balance");
}
```

- **Always use the member initializer list.** Members get *initialized* there.
  Assigning in the body happens *after* default construction. `const` and
  reference members *must* be initialized in the list.
- Members are initialized in **declaration order**, not list order. Keep them
  consistent (`-Wreorder` warns you).
- **`explicit`** on single-argument constructors prevents surprise implicit
  conversions (`BankAccount a = "bob";` would otherwise compile!).
- Default member initializers (`double balance_{0.0};`) give a default for every constructor.
- `= default` asks the compiler for the default version. `= delete` forbids a function.

### Special member functions
The compiler can generate these for you:

| Function | Signature | When called |
|----------|-----------|-------------|
| default ctor | `T()` | `T t;` |
| destructor | `~T()` | when t goes out of scope |
| copy ctor | `T(const T&)` | `T b = a;` |
| copy assignment | `T& operator=(const T&)` | `b = a;` |
| move ctor | `T(T&&)` | `T b = std::move(a);` (ch. 9) |
| move assignment | `T& operator=(T&&)` | `b = std::move(a);` |

The **Rule of Zero**: design classes so you never write these yourself. Use
members that already manage themselves (`std::string`, `std::vector`,
`std::unique_ptr`). Chapter 5 covers the Rule of Three/Five for when you can't.

## 3. Destructors

`~T()` runs automatically and **deterministically** when an object's lifetime
ends: at the end of its scope, at `delete`, or when its container is destroyed.
This is the foundation of RAII (ch. 5). Objects are destroyed in **reverse order**
of construction.

## 4. `const` member functions

```cpp
double balance() const;   // promises not to modify *this
```
Only `const` member functions can be called on `const` objects or through
`const&`. Since you pass objects by `const&` all the time, **mark every
non-mutating method `const`**. If a member must change inside a const
method (a cache, or a mutex), declare it `mutable`.

## 5. `this` and static members

- Inside a member function, `this` is a pointer to the current object.
- `static` data members are shared by all instances. `static` member functions
  have no `this`.

```cpp
class Widget {
    static inline int count_ = 0;     // C++17 inline static: defined right here
public:
    Widget() { ++count_; }
    static int count() { return count_; }
};
```

## 6. Operator overloading

Make your types behave like built-ins when it is **natural**:

```cpp
struct Vec2 {
    double x{}, y{};
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    friend Vec2 operator+(Vec2 a, const Vec2& b) { return a += b; }  // symmetric → free/friend
    bool operator==(const Vec2&) const = default;                    // C++20: generated
    auto operator<=>(const Vec2&) const = default;                   // all of <,>,<=,>=
};
std::ostream& operator<<(std::ostream& os, const Vec2& v) { return os << '(' << v.x << ", " << v.y << ')'; }
```

Guidelines: implement `+` in terms of `+=`, and return `*this` by reference
from compound assignment. Binary symmetric operators should be non-members.

## 7. Inheritance & polymorphism

```cpp
class Shape {
public:
    virtual ~Shape() = default;            // ← REQUIRED for polymorphic base classes
    virtual double area() const = 0;       // pure virtual → Shape is abstract
    virtual std::string name() const { return "shape"; }
};

class Circle : public Shape {
public:
    explicit Circle(double r) : r_{r} {}
    double area() const override { return std::numbers::pi * r_ * r_; }
    std::string name() const override { return "circle"; }
private:
    double r_;
};

std::vector<std::unique_ptr<Shape>> shapes;
shapes.push_back(std::make_unique<Circle>(1.0));
for (const auto& s : shapes) std::cout << s->area();   // dynamic dispatch via vtable
```

- `virtual` means "look up the function at runtime from the object's real type".
- `override` makes the compiler verify that you really override something.
  **Always write it.**
- `final` prevents further overriding or inheritance.
- **Virtual destructor**: deleting a `Derived` through a `Base*` without one is UB.
- **Object slicing**: `Shape s = circle;` copies only the `Shape` part. Use
  polymorphic objects through pointers or references.

### How virtual calls work
Each polymorphic class has a **vtable** (a table of function pointers). Each
object carries a hidden **vptr** to its class's vtable. `s->area()` becomes
"follow vptr, index the table, call". The overhead is small, but the call
can't be inlined, and objects grow by one pointer.

### Access in inheritance
`public` inheritance models **is-a**. Use **composition** (a member) for
**has-a**. Favor composition. Deep hierarchies get rigid.

## 8. Interfaces

C++ has no `interface` keyword. An *interface* is a class with only pure
virtual functions and a virtual destructor. That's exactly how our CRUD API
defines `TaskRepository` in part 3.

## 9. Friends

`friend` grants a function or class access to private members. Use it
sparingly, mostly for operators like `<<`.

## Pitfalls
| Pitfall | Fix |
|---------|-----|
| Missing virtual destructor in a base | `virtual ~Base() = default;` |
| Forgetting `override` (typo creates a new function) | always write `override` |
| Slicing when storing derived objects by value | store `unique_ptr<Base>` |
| Calling virtual functions in constructors | the derived part doesn't exist yet: avoid |
| Non-const getters | mark them `const` |
| Implicit conversions from 1-arg ctors | `explicit` |

## Examples
| File | Shows |
|------|-------|
| `examples/01_class_basics.cpp` | BankAccount: encapsulation, invariants, const methods |
| `examples/02_constructors_lifetime.cpp` | ctor/dtor order, copy, explicit, static counters |
| `examples/03_operator_overloading.cpp` | `Vec2` arithmetic, `<=>`, `<<` |
| `examples/04_inheritance_polymorphism.cpp` | Shape hierarchy, vtables, `override`, `final` |
| `examples/05_composition_interfaces.cpp` | interfaces + composition (Logger, Notifier) |
