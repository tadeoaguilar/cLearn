# 08 — Error Handling

> Goal: know the tools (exceptions, `std::optional`, `std::expected`, error
> codes and assertions), and **when** to use each one.

## 1. Kinds of "errors"

| Situation | Example | Tool |
|-----------|---------|------|
| **Programming bug** (violated precondition) | index out of range, null deref | `assert`, contracts, crash fast |
| **Exceptional, unrecoverable locally** | out of memory, DB connection lost | **exceptions** |
| **Expected failure, caller decides** | "user not found", parse error | `std::optional` / `std::expected` |
| **Low-level / C APIs / no-exceptions builds** | `errno`, `std::error_code` | error codes |

Unreal Engine and many game engines compile **without exceptions**. They use
return values, `check()` macros, and logs. Server code (like our CRUD API)
typically uses exceptions internally and translates them to HTTP status codes
at the boundary.

## 2. Exceptions

```cpp
#include <stdexcept>

double safe_div(double a, double b) {
    if (b == 0) throw std::invalid_argument("division by zero");
    return a / b;
}

try {
    safe_div(1, 0);
} catch (const std::invalid_argument& e) {   // catch by const reference!
    std::cerr << "bad argument: " << e.what() << '\n';
} catch (const std::exception& e) {          // more general handlers go later
    std::cerr << "error: " << e.what() << '\n';
} catch (...) {                              // anything at all
    std::cerr << "unknown error\n";
}
```

### The standard hierarchy (partial)
```
std::exception
 ├── std::logic_error        (bugs that could have been detected before running)
 │    ├── std::invalid_argument
 │    ├── std::out_of_range
 │    └── std::domain_error
 ├── std::runtime_error      (things only detectable at runtime)
 │    ├── std::overflow_error / underflow_error / range_error
 │    └── std::system_error  (wraps std::error_code)
 └── std::bad_alloc, std::bad_cast, std::bad_optional_access, ...
```

### Custom exceptions
```cpp
class NotFoundError : public std::runtime_error {
public:
    explicit NotFoundError(const std::string& what) : std::runtime_error(what) {}
};
```

### Stack unwinding: why RAII matters
When an exception propagates, every local object in each exited scope is
destroyed. RAII objects (vectors, smart pointers, lock guards, files) clean up
automatically. Raw `new` and manual `unlock()` do not.

### Exception safety guarantees
| Guarantee | Promise |
|-----------|---------|
| **No-throw** (`noexcept`) | never throws (destructors, swaps, moves should be here) |
| **Strong** | if it throws, state is unchanged (commit-or-rollback) |
| **Basic** | if it throws, no leaks and invariants hold, but state may change |
| none | anything goes. Avoid. |

**Copy-and-swap** gives you the strong guarantee: do all the risky work on a
copy, then `swap` it in, which is `noexcept`.

### `noexcept`
Marks a function as non-throwing. If it throws anyway, `std::terminate` is
called. **Mark move constructors `noexcept`**. Otherwise `std::vector` copies
instead of moving when it grows, because it needs the strong guarantee.

### Rules of thumb
- Throw by value, catch by `const&`.
- Never throw from destructors.
- Don't use exceptions for normal control flow (like "end of loop").
- Catch where you can actually *handle* the error, or at a boundary such as
  `main`, a request handler or a thread entry point.

## 3. `std::optional<T>`: maybe a value

```cpp
std::optional<User> find_user(int id) {
    if (auto it = users.find(id); it != users.end()) return it->second;
    return std::nullopt;
}

if (auto u = find_user(42)) {      // contextual bool
    std::cout << u->name;          // -> and * access the value
}
auto name = find_user(7).value_or(User{"guest"}).name;
find_user(7).value();              // throws std::bad_optional_access if empty
```
Use it when "no result" is a normal outcome and needs no explanation.

## 4. `std::expected<T, E>` (C++23): a value **or** an error

```cpp
enum class ParseError { Empty, NotANumber, OutOfRange };

std::expected<int, ParseError> parse_int(std::string_view s) {
    if (s.empty()) return std::unexpected(ParseError::Empty);
    ...
    return value;
}

auto r = parse_int("42");
if (r) use(*r);
else   report(r.error());

// Monadic chaining (C++23): and_then / transform / or_else
auto doubled = parse_int(s).transform([](int x) { return x * 2; });
```
This is like Rust's `Result` or Swift's `Result`. Use it when the caller needs
to know **why** something failed and the failure is expected (validation,
parsing, I/O).

## 5. Error codes

```cpp
#include <system_error>
std::error_code ec;
std::filesystem::remove("missing.txt", ec);   // non-throwing overload
if (ec) std::cerr << ec.message();
```
The standard library offers throwing **and** `error_code` overloads for
filesystem and some networking APIs. C libraries return ints and set `errno`.

## 6. Assertions

```cpp
#include <cassert>
assert(index < size && "index out of range");    // removed when NDEBUG is defined (release builds)
static_assert(sizeof(int) == 4, "need 32-bit int"); // compile time, always on
```
Assertions document and check **assumptions about your own code**. They are
not for validating user input.

## 7. Putting it together: layered error handling (preview of the API)

```
HTTP handler      ← catches exceptions/expected errors → 400/404/500 responses
   │
Service layer     ← validates input → std::expected<Task, ValidationError>
   │
Repository (DB)   ← throws DatabaseError on connection loss; returns optional for "not found"
```

## Examples
| File | Shows |
|------|-------|
| `examples/01_exceptions.cpp` | throw/catch, hierarchy, rethrow, custom exceptions, unwinding |
| `examples/02_optional.cpp` | lookups, `value_or`, monadic `and_then`/`transform` |
| `examples/03_expected.cpp` | parsing & validation with `std::expected` and chaining |
| `examples/04_error_codes_assert.cpp` | `std::error_code`, filesystem, `assert`, `static_assert` |
| `examples/05_exception_safety.cpp` | basic vs strong guarantee, `noexcept` move & vector |
