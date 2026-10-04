# 22 — Exercises: Memory Management in C

Build every solution with `-fsanitize=address,undefined -g` at least once
and make sure it runs clean. On Linux, LeakSanitizer must report zero leaks;
on macOS, use `leaks --atExit -- ./prog` (built without ASan).

### Ex 1 — String builder ⭐⭐
A growable string, `StrBuf { char* data; size_t len, cap; }`:
- `sb_init`, `sb_free`, `sb_append(sb, const char*)`, `sb_append_char`
- `sb_appendf(sb, fmt, ...)`: printf-style. Use `va_list` and call `vsnprintf` twice:
  once with size 0 to measure, then again after growing.
- `char* sb_take(StrBuf*)`: hands the heap string to the caller (who must
  `free` it) and leaves the builder empty.
- `data` must always be NUL-terminated, even when empty.

Build a 1000-row CSV in memory with it and print the first 3 lines and the total length.
→ `solutions/ex01_string_builder.c`

### Ex 2 — A type-generic vector with macros ⭐⭐
Write a macro `DEFINE_VEC(T, Name)` that generates a struct `Name` and
functions `Name##_push`, `Name##_get`, `Name##_free` for element type `T`,
like a C++ template instantiation. Instantiate it for `int`, `double` and a
`Point` struct, and use all three.
→ `solutions/ex02_generic_vec.c`

### Ex 3 — Hash map with owned keys ⭐⭐⭐
`StrIntMap`: string → int with separate chaining. Requirements:
- The map **copies** keys (`strdup`-style) and owns them.
- `map_put` inserts or updates, `map_get` returns `bool` + an out-param, `map_remove` frees the node and its key.
- Grow (rehash) when the load factor exceeds 0.75.
- `map_free` releases every node and key, then the bucket array.

Use it to count word frequencies in a paragraph, then remove a few words.
Verify zero leaks.
→ `solutions/ex03_hash_map.c`

### Ex 4 — Write your own `malloc` ⭐⭐⭐⭐
Implement `my_malloc(size)` / `my_free(p)` over a **static** 64 KB array (no
calls to the real malloc):
- Each block has a header with its size and a "free" flag. The block list is
  implicit: the next header is at `(char*)(header + 1) + size`. (Hint: if sizes
  are multiples of 16, the low bit of `size` is always 0 and can hold the flag.)
- **First fit**: walk the blocks and take the first free one that is big enough.
- **Split** a large free block if the remainder can hold a header + 16 bytes.
- **Coalesce** on free: merge with the following free block (bonus: the previous
  one too, which needs a footer or a previous-size field).
- Every returned pointer must be 16-byte aligned.
- `heap_dump()` prints the block list.

Show fragmentation: allocate A B C D, free B and C, and check that they merge
and that a larger request fits in the merged space.
→ `solutions/ex04_my_malloc.c`

### Ex 5 — Leak detector ⭐⭐
Write `dbg_malloc(size, file, line)` / `dbg_free(p, file, line)` and the
macros `MALLOC(n)` / `FREE(p)` that pass `__FILE__` and `__LINE__`. Keep a
table of live allocations. At exit (`atexit`), print every leaked block with
the file and line that allocated it. Also detect `FREE` of an unknown pointer
(a double free or a foreign pointer).
→ `solutions/ex05_leak_detector.c`
