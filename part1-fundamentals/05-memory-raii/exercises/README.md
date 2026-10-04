# 05 — Exercises: Memory & RAII

### Ex 1 — ScopeTimer and ScopeGuard ⭐⭐
1. `ScopeTimer`: the constructor takes a label and records `std::chrono::steady_clock::now()`.
   The destructor prints how many microseconds elapsed.
2. `ScopeGuard`: stores a `std::function<void()>` and calls it in the
   destructor, unless `dismiss()` was called. Use it to "roll back" a change
   if a function throws.
→ `solutions/ex01_scope_guards.cpp`

### Ex 2 — Linked list with `unique_ptr` ⭐⭐
Build a singly linked `IntList` where each node owns the next one
(`std::unique_ptr<Node> next`). Support `push_front`, `pop_front`, `size`,
`reverse` (in place, re-linking only, without copying values) and printing.
**Bonus:** the default destructor recurses once per node and can overflow the
stack for a million nodes. Write an iterative destructor.
→ `solutions/ex02_unique_list.cpp`

### Ex 3 — Asset cache with `weak_ptr` ⭐⭐⭐
`AssetCache::get(name)` returns `std::shared_ptr<Asset>`. The cache stores
only `std::weak_ptr`, so an asset is freed when no one uses it, and loaded
again on the next request. Count the loads to prove that sharing works.
→ `solutions/ex03_asset_cache.cpp`

### Ex 4 — Write your own `UniquePtr<T>` ⭐⭐⭐
A minimal move-only smart pointer: a constructor from `T*`, a destructor,
deleted copy, move constructor and assignment, `operator*`, `operator->`,
`get()`, `release()`, `reset()`, `explicit operator bool`. Also add a
`make_unique_ptr<T>(args...)` helper. (It uses a small template; peek at ch. 7
if needed.)
→ `solutions/ex04_my_unique_ptr.cpp`
