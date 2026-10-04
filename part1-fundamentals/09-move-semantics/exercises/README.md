# 09 — Exercises: Move Semantics

### Ex 1 — Your own String (Rule of Five) ⭐⭐⭐
Write `class String` that owns a heap `char*` buffer (null-terminated) and a
size. Implement: a constructor from `const char*`, the destructor, copy
constructor and copy assignment (deep copy), and move constructor and move
assignment (steal, `noexcept`). Add `operator+=`, `size()`, `c_str()` and `<<`.
Keep a static counter of heap allocations, and show that moves allocate nothing.
→ `solutions/ex01_string_rule_of_five.cpp`

### Ex 2 — Move-only handle ⭐⭐
Simulate an OS resource: `int open_resource()` / `void close_resource(int)`
print and track the open handles. Wrap them in a move-only `Handle` class.
Copying must be deleted. Moving transfers the id, and the source becomes
`-1`. Put handles in a `std::vector<Handle>`, move one out into a function that
"consumes" it, and verify that every handle is closed exactly once.
→ `solutions/ex02_move_only_handle.cpp`

### Ex 3 — Registry with perfect forwarding ⭐⭐
`template <typename T> class Registry` stores `std::vector<std::unique_ptr<T>>`.
`template <typename... Args> T& create(Args&&... args)` forwards the arguments
to `T`'s constructor and returns a reference to the new object. Test it with a
type whose constructor takes a `std::string` (an rvalue) and an `int&` (an lvalue
that the constructor modifies, to prove it was forwarded as a reference).
→ `solutions/ex03_registry_forwarding.cpp`

### Ex 4 — Spot the bug ⭐
Each snippet in `solutions/ex04_spot_the_bug.cpp` contains a move-related
mistake (use-after-move, `std::move` on const, moving a returned local,
missing `noexcept`). Read the comments, predict the behavior, and run it.
→ `solutions/ex04_spot_the_bug.cpp`
