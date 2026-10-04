# 06 — The Standard Library: Containers, Iterators & Algorithms

> Goal: stop writing loops by hand for everything. Pick the right container,
> and learn the algorithms that make code shorter, faster and harder to get wrong.

## 1. The three pillars

```
 Containers ── store data          vector, map, unordered_map, set, deque, array...
     │
 Iterators ─── generalized pointers  begin(), end(), ++it, *it
     │
 Algorithms ── operate on ranges   sort, find, count_if, transform, accumulate...
```
Algorithms don't know about containers. They work on **iterator ranges**
`[first, last)`. That's why `std::sort` works on a `vector`, an `array` and a
C array alike.

## 2. Sequence containers

| Container | Layout | Access | Insert/erase | Notes |
|-----------|--------|--------|--------------|-------|
| `std::vector<T>` | contiguous, growable | O(1) random | O(1) amortized at end, O(n) middle | **default choice** |
| `std::array<T,N>` | contiguous, fixed | O(1) | — | size known at compile time; lives on stack |
| `std::deque<T>` | chunks | O(1) | O(1) at both ends | queues |
| `std::list<T>` | doubly linked | O(n) | O(1) given an iterator | rarely the right choice (cache-unfriendly) |
| `std::string` | contiguous chars | O(1) | like vector | |

### `vector` in depth
```cpp
std::vector<int> v;
v.reserve(100);         // pre-allocate capacity (avoid repeated reallocations)
v.push_back(1);         // copy/move in
v.emplace_back(2);      // construct in place from ctor args
v.size(); v.capacity(); v.empty();
v[0];                   // unchecked
v.at(0);                // bounds-checked (throws std::out_of_range)
v.front(); v.back();
v.pop_back();
v.insert(v.begin() + 1, 42);
v.erase(v.begin());     // O(n): shifts everything after
std::erase(v, 42);      // C++20: remove all 42s
std::erase_if(v, pred);
v.clear();
```
When `size == capacity`, `push_back` allocates a bigger block (usually 1.5–2×),
moves the elements over, and frees the old block. **All pointers, references and
iterators into the vector become invalid.**

## 3. Associative containers

| Container | Ordered? | Lookup | Implementation |
|-----------|----------|--------|----------------|
| `std::map<K,V>` | yes (by key) | O(log n) | red-black tree |
| `std::unordered_map<K,V>` | no | O(1) average | hash table |
| `std::set<T>` / `unordered_set<T>` | as above | as above | unique keys only |
| `std::multimap`, `multiset` | | | duplicate keys allowed |

```cpp
std::unordered_map<std::string, int> ages;
ages["ada"] = 36;                 // inserts default (0) if missing, then assigns
ages.insert({"bob", 20});         // doesn't overwrite existing
ages.insert_or_assign("bob", 21);
ages.try_emplace("cy", 40);       // only constructs if key missing
if (auto it = ages.find("ada"); it != ages.end()) it->second++;
if (ages.contains("bob")) {}      // C++20
ages.erase("bob");
for (const auto& [name, age] : ages) {}  // structured bindings
```
⚠️ `operator[]` **inserts** missing keys, so you can't use it on a `const` map.
Use `find` or `at` for read-only lookups.

Choose `map` when you need sorted iteration or range queries
(`lower_bound`). Choose `unordered_map` for pure lookups. Custom key types
need `operator<` for `map`, or a hash and `==` for `unordered_map`.

## 4. Container adapters
- `std::stack<T>`: LIFO (`push`, `pop`, `top`)
- `std::queue<T>`: FIFO (`push`, `pop`, `front`)
- `std::priority_queue<T>`: max-heap by default; `std::greater<T>` makes it a min-heap

## 5. Iterators

```cpp
for (auto it = v.begin(); it != v.end(); ++it) std::cout << *it;
auto rit = v.rbegin();          // reverse
auto cit = v.cbegin();          // const
std::next(it, 3); std::prev(it); std::distance(a, b);
```
Categories (weakest → strongest): input → forward → bidirectional
(`list`, `map`) → random access (`deque`) → contiguous (`vector`, `array`).
`std::sort` needs random access, which is why you can't sort a `std::list`
with it (use `list.sort()`).

## 6. Algorithms: the essential list

```cpp
#include <algorithm>  #include <numeric>
// Searching
std::find(b, e, value);          std::find_if(b, e, pred);
std::count(b, e, value);         std::count_if(b, e, pred);
std::any_of / all_of / none_of(b, e, pred);
std::binary_search(b, e, v);     std::lower_bound(b, e, v);  // on SORTED ranges
// Ordering
std::sort(b, e);                 std::sort(b, e, comp);
std::stable_sort(b, e, comp);    std::partial_sort, std::nth_element
std::min_element / max_element / minmax_element
// Transforming
std::transform(b, e, out, f);    std::copy_if(b, e, out, pred);
std::reverse(b, e);              std::unique(b, e);  // removes ADJACENT dups
std::fill, std::iota, std::shuffle
// Reducing (<numeric>)
std::accumulate(b, e, init);     std::reduce (parallelizable)
std::inner_product, std::partial_sum
```

### Ranges (C++20): the modern spelling
```cpp
std::ranges::sort(v);                        // no begin/end needed
std::ranges::sort(people, {}, &Person::age); // projection: sort by age
auto it = std::ranges::find(people, "Ada", &Person::name);
auto evens = v | std::views::filter([](int x){ return x % 2 == 0; })
               | std::views::transform([](int x){ return x * x; });   // lazy!
```
Chapter 10 covers views in depth.

### The erase-remove idiom (pre-C++20)
`std::remove` doesn't remove anything. It shuffles the kept elements to the
front and returns the new logical end. You then `erase` the tail:
```cpp
v.erase(std::remove_if(v.begin(), v.end(), pred), v.end());
std::erase_if(v, pred);   // C++20 does both
```

## 7. Choosing a container: a decision guide

```
Need key → value lookup?
 ├─ yes → need sorted order / range queries? ── yes → std::map
 │                                           └─ no  → std::unordered_map
 └─ no  → size fixed at compile time? ── yes → std::array
                                     └─ no  → need fast push/pop at FRONT? ── yes → std::deque
                                                                         └─ no  → std::vector ✅
```

## 8. Complexity matters (Big-O cheat sheet)

| Operation | vector | deque | list | map | unordered_map |
|-----------|--------|-------|------|-----|---------------|
| index `[i]` | O(1) | O(1) | — | — | — |
| find by value | O(n) | O(n) | O(n) | O(log n) | O(1) avg |
| push_back | O(1)* | O(1) | O(1) | — | — |
| insert middle | O(n) | O(n) | O(1) | O(log n) | O(1) avg |

\* amortized. In practice, `vector` often beats theoretically better
structures for small to medium n, because contiguous memory is cache-friendly.

## Examples
| File | Shows |
|------|-------|
| `examples/01_vector.cpp` | growth/capacity, emplace, erase, invalidation |
| `examples/02_maps_sets.cpp` | map vs unordered_map, word frequency, custom keys & hashes |
| `examples/03_adapters.cpp` | stack, queue, priority_queue (task scheduler) |
| `examples/04_algorithms.cpp` | the classic algorithms in action |
| `examples/05_ranges.cpp` | `std::ranges` algorithms and projections |
