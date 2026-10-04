# 06 — Exercises: STL

Try to write these **without raw index loops** where an algorithm fits.

### Ex 1 — Top-N words ⭐⭐
Given a paragraph, count the words case-insensitively and ignoring punctuation.
Print the 5 most frequent, breaking ties alphabetically. Use `unordered_map`
for counting, then copy into a `vector<pair<...>>` and `partial_sort`.
→ `solutions/ex01_top_words.cpp`

### Ex 2 — Group anagrams ⭐⭐
Group `{"listen","silent","enlist","google","gooegl","cat","act","tac","dog"}`
into anagram families. The key is the word with its letters sorted. Print the
groups ordered by size (largest first).
→ `solutions/ex02_anagrams.cpp`

### Ex 3 — Grade report ⭐⭐
`struct Student { std::string name; std::vector<int> scores; };`
Using algorithms (`accumulate`, `sort`, `partition`, `max_element`…):
compute each average, rank the students, list who's failing (avg < 60), find
the top scorer of each assignment, and print the class median.
→ `solutions/ex03_grades.cpp`

### Ex 4 — LRU cache ⭐⭐⭐
Implement `LRUCache<int, std::string>` with capacity N: `get(key)` returns
`std::optional<V>` and marks the key as most recently used. `put(key, value)`
evicts the least recently used entry when full. Both must be **O(1)**. Hint:
`std::list` of key/value pairs, plus an `unordered_map<Key, list::iterator>`.
`list::splice` moves a node without invalidating iterators.
→ `solutions/ex04_lru_cache.cpp`

### Ex 5 — Maze solver (BFS) ⭐⭐⭐
Given a maze as `std::vector<std::string>` with `S` (start), `E` (end), `#`
(wall) and `.` (floor), find the shortest path with breadth-first search
(`std::queue`). Reconstruct it from a "came from" map and draw it with `*`.
→ `solutions/ex05_maze_bfs.cpp`
