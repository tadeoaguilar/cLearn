# 07 — Exercises: Templates & Concepts

### Ex 1 — Generic helpers ⭐
- `template <std::totally_ordered T> const T& clamp_to(const T& v, const T& lo, const T& hi)`
- `find_index(const Range& r, const T& value)`, which returns `std::optional<std::size_t>`
- `print_all(const std::ranges::range auto& r)`, which prints `[a, b, c]`
Test them with ints, doubles, strings, vectors and arrays.
→ `solutions/ex01_generic_helpers.cpp`

### Ex 2 — Compile-time sized Matrix ⭐⭐⭐
`template <typename T, std::size_t R, std::size_t C> class Matrix` backed by
`std::array<T, R*C>`. Implement `operator()(r, c)`, `+`, `transpose()` (it
returns `Matrix<T, C, R>`!), and multiplication `Matrix<T,R,C> * Matrix<T,C,K> → Matrix<T,R,K>`.
Multiplying mismatched sizes must **fail to compile**. Add an `identity()`
static factory that only exists for square matrices (`requires (R == C)`).
→ `solutions/ex02_matrix.cpp`

### Ex 3 — Ring buffer ⭐⭐
`RingBuffer<T, N>` keeps the last N pushed values. Provide `push`, `size`,
`full`, `operator[](i)` (where 0 = oldest) and `for_each(F f)`. Use it to
compute a moving average of a sensor stream.
→ `solutions/ex03_ring_buffer.cpp`

### Ex 4 — A `Serializable` concept ⭐⭐
Define `concept Serializable` that accepts types with a member
`std::string serialize() const`. Write `std::string to_text(const T&)` that
handles three cases: `Serializable` types, arithmetic types (`std::to_string`)
and `std::string`. Then write `save_all(const std::vector<T>&)` that joins
them with newlines. This idea comes back in ch. 13.
→ `solutions/ex04_serializable.cpp`

### Ex 5 — Variadic utilities ⭐⭐
- `min_of(a, b, c, ...)` with a fold expression; all arguments must be the same type (`static_assert`)
- `make_string(args...)` builds a `std::string` via `std::ostringstream`
- `count_if_args(pred, args...)` counts how many args satisfy the predicate
→ `solutions/ex05_variadic.cpp`
