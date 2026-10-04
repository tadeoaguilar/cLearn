# 22 — Memory Management in C

> Goal: allocate, own and free memory correctly with no RAII to help. Then
> build the allocators that real systems (game engines, compilers, kernels,
> databases) use instead of calling `malloc` for every object.

In C++ chapter 5 you learned that destructors free memory for you. In C,
**nothing** happens automatically except for stack variables. Every
`malloc` needs exactly one matching `free`, on every path, written by you.

## 1. Where things live

```
high addresses ┌──────────────────────────────┐
               │ stack  ↓  locals, return addr │  automatic; ~8 MB; freed on return
               │                              │
               │ memory-mapped region         │  shared libs, big malloc() blocks (mmap)
               │                              │
               │ heap   ↑  malloc/calloc      │  manual; you call free()
               ├──────────────────────────────┤
               │ .bss   zero-initialized globals  static int counter;
               │ .data  initialized globals       static int limit = 10;
               │ .rodata string literals, const   "hello"
               │ .text  machine code               main, printf
low addresses  └──────────────────────────────┘
```

`examples/01_memory_map.c` prints a real address from each region. ASLR
(address-space layout randomization) moves the regions on every run.

## 2. The allocation API (`<stdlib.h>`)

| Function | Does | Returns on failure |
|----------|------|--------------------|
| `malloc(size)` | allocates `size` bytes, **uninitialized** | `NULL` |
| `calloc(n, size)` | allocates `n*size` bytes, **zeroed**, and checks `n*size` for overflow | `NULL` |
| `realloc(p, size)` | resizes; may **move** the block (copying the contents) | `NULL`, and `p` is **still valid** |
| `free(p)` | releases; `free(NULL)` is a no-op | — |
| `aligned_alloc(align, size)` | like malloc with a stronger alignment (C11); `size` must be a multiple of `align` | `NULL` |

### Idioms to memorize

```c
T* p = malloc(sizeof *p);                 // sizeof *p, not sizeof(T): survives type changes
T* arr = malloc(n * sizeof *arr);         // DANGER if n is huge: n*size can overflow
T* arr = calloc(n, sizeof *arr);          // safe: calloc checks the multiplication
if (!arr) { /* handle out-of-memory */ }

// realloc: NEVER write p = realloc(p, ...). On failure you'd lose (and leak) the old block.
T* tmp = realloc(arr, new_n * sizeof *arr);
if (!tmp) { /* arr is still valid and still yours */ }
else arr = tmp;

free(arr);
arr = NULL;                               // defuses double frees and use-after-free
```

## 3. Ownership: the rules you write in comments

The compiler doesn't track ownership in C, so you document it, and
consistent naming makes it visible:

| Convention | Meaning | Example |
|------------|---------|---------|
| `x_create()` / `x_destroy()` | the callee allocates the object; the caller must destroy it | `fopen`/`fclose` |
| `x_init(X*)` / `x_free(X*)` | the **caller** provides the storage (stack, array, inside another struct); init/free manage what it points to | `pthread_mutex_init`/`_destroy` |
| caller-allocated buffer | `int fmt(char* buf, size_t size, ...)`: no allocation at all | `snprintf`, `strerror_r` |
| returns `char*` "caller frees" | a heap string you now own | `strdup`, `getline` |
| returns `const char*` | borrowed: valid until X; don't free it | `getenv`, `strerror` |

`x_init` and `x_free` are the most flexible pair. The caller decides where the
object lives (on the stack, inside an array, inside another struct) and only
the contents are heap-managed. That is what C++'s constructors and destructors
do automatically.

A borrowed pointer must never outlive its owner. In C++ that becomes a
dangling reference; in C it is just as real, but no tool warns you at
compile time.

## 4. The classic bugs (and the tools that catch them)

| Bug | What happens | Caught by |
|-----|--------------|-----------|
| **leak**: lose the last pointer without `free` | memory grows until the process dies | LeakSanitizer (Linux), `leaks` (macOS), Valgrind |
| **double free** | heap metadata corruption, often a crash later | ASan, glibc/macOS malloc checks |
| **use after free** | reads garbage or someone else's data; **security hole** | ASan |
| **buffer overflow** (off-by-one) | overwrites neighbouring objects or heap metadata | ASan |
| **uninitialized read** (`malloc` isn't zeroed) | random behaviour | MemorySanitizer (clang/Linux), Valgrind |
| `free` of a non-heap pointer / of an interior pointer | crash | ASan |

**AddressSanitizer** is the single most valuable tool for C. Use it in every debug build:

```bash
cc -std=c17 -g -fsanitize=address,undefined -fno-omit-frame-pointer examples/07_memory_bugs.c -o bugs
./bugs overflow     # ASan prints the exact line of the bad access and where the block was allocated
# Linux: leaks are reported at exit automatically (LeakSanitizer).
# macOS: ASan's leak checker isn't supported. Use Apple's tool instead:
MallocStackLogging=1 leaks --atExit -- ./bugs leak | grep LEAK   # build without ASan for this
# Linux alternative without recompiling:  valgrind --leak-check=full ./bugs leak
```

## 5. Dynamic arrays (your own `std::vector`)

```c
typedef struct { int* data; size_t len, cap; } IntVec;
```

Grow the capacity **geometrically** (×2 or ×1.5), never by a constant. Then
`n` pushes cost O(n) in total (amortized O(1) each) instead of O(n²). See
`examples/04_dynamic_array.c`, which logs every `realloc`.

## 6. Custom allocators: why `malloc` everywhere is slow

`malloc` is general-purpose. It is thread-safe, handles every size, and
fights fragmentation. You pay for that with ~20–100 ns per call, 8–16 bytes of
header per block, and objects scattered across memory (cache misses). When
you know your allocation pattern, a custom allocator wins easily.

### Arena (a.k.a. bump / linear / region allocator)

```
 ┌────────────────────────── one big block ───────────────────────────┐
 │ obj A │ obj B │pad│ obj C │            free                        │
 └───────────────────────────┴─────────────────────────────────────────┘
                             ↑ offset: the next allocation goes here
```

- `alloc` = round up the offset for alignment, then bump it. About **2 instructions**.
- There is **no individual free**: you `reset` or `destroy` the whole arena at once.
- It fits "phase" lifetimes: everything for one frame of a game, one HTTP
  request, or one compiler pass. One `free` instead of thousands, and no
  leaks within the phase are possible.

### Pool (fixed-size block allocator)

```
 ┌─────┬─────┬─────┬─────┬─────┬─────┐  all blocks have the same size
 │ used│  ●──┼──►● │ used│  ●──┼──► NULL   free blocks form a linked list stored
 └─────┴─────┴─────┴─────┴─────┴─────┘     INSIDE the free blocks themselves
   free_list ─────┘
```

- `alloc` pops the head of the free list; `free` pushes onto it. Both are O(1)
  and there is no fragmentation.
- It suits many objects of one type that are created and destroyed in any
  order: particles, network connections, tree nodes.

### Alignment

Every type has an alignment requirement (`alignof(double) == 8` on most
platforms). Misaligned access is UB in C. It is slow on x86 and can **fault**
on some ARM cores. Round an offset up to a power-of-two alignment `a` with:

```c
size_t aligned = (offset + (a - 1)) & ~(a - 1);   // e.g. offset 13, a 8 → 16
```

`malloc` returns memory aligned for any standard type
(`alignof(max_align_t)`, usually 16). Custom allocators must guarantee this
themselves.

## 7. What `malloc` does under the hood (simplified)

```
 ┌────────┬───────────────┐┌────────┬─────────┐┌────────┬──────────────────┐
 │ size=32│ user data ... ││size=16 │ (free)  ││size=64 │ user data ...    │
 │ used=1 │               ││used=0  │ next ●──┼┼► ...                     │
 └────────┴───────────────┘└────────┴─────────┘└────────┴──────────────────┘
   header ↑ p returned to you points just after the header
```

- Each block has a hidden **header** with its size, which is how `free(p)`
  knows how much to release. Writing past your block corrupts the *next*
  header, and that is why heap overflows crash later in unrelated code.
- Free blocks are kept in lists (bins by size). Adjacent free blocks are
  **coalesced** to fight fragmentation.
- The memory comes from the OS in large chunks: `brk`/`sbrk` (grow the heap)
  or `mmap` (big allocations, typically ≥128 KB, returned directly to the OS
  on `free`).

You build exactly this in exercise 4.

## Examples

| File | Shows |
|------|-------|
| `01_memory_map.c` | addresses from text, rodata, data, bss, heap and stack |
| `02_malloc_family.c` | `malloc` vs `calloc`, the safe `realloc` pattern, overflow-checked sizes, `aligned_alloc` |
| `03_ownership.c` | create/destroy vs init/free, caller-allocated buffers, transferring ownership |
| `04_dynamic_array.c` | a growable `IntVec`, geometric growth, shrink-to-fit |
| `05_arena.c` | a bump allocator with alignment, a temp scope, and a per-frame reset |
| `06_pool.c` | a fixed-size pool with an intrusive free list |
| `07_memory_bugs.c` | intentionally broken code: run it under ASan to see each bug reported |

## Pitfalls

- `sizeof(p)` where `p` is a pointer gives 8, not the buffer size. Pass
  sizes explicitly.
- `malloc(strlen(s))` forgets the `'\0'`: use `strlen(s) + 1`.
- Returning a pointer to a local array: the stack frame is gone after `return`.
- `free` doesn't set the pointer to `NULL`. Other copies of that pointer still dangle.
- A struct with pointer members needs a *deep* free (free the members, then
  the struct), in reverse order of creation.
- Mixing allocators: memory from `aligned_alloc`, `malloc` or an arena must
  be released by its own counterpart.

➡️ Next: [chapter 23](../23-hardware-controller/README.md), where the
"memory" you write to is a hardware device.
