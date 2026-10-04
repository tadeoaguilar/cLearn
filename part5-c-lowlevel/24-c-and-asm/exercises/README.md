# 24 — Exercises: Combining C and Assembly

Every assembly exercise needs both an x86-64 and an AArch64 version, selected
with `#if defined(__x86_64__)` / `#elif defined(__aarch64__)`. Write the one
for your machine first. Check the other one with Compiler Explorer, or
cross-assemble it:
`clang --target=aarch64-linux-gnu -c file.S` / `clang --target=x86_64-linux-gnu -c file.S`.

Build a `.c` + `.S` pair with: `cc -std=c17 -Wall ex02_mem.c ex02_mem.S -o ex02`.

### Ex 1 — Rotate with inline asm ⭐⭐
Write `uint32_t rotl32_asm(uint32_t x, unsigned n)` with inline asm (x86:
`roll %cl, reg`, so the count must be in `cl`: use the `"c"` constraint;
AArch64 has only `ror`, so rotate right by `32 - n`). Write the portable C
version `(x << n) | (x >> (-n & 31))` and verify that both agree for
100,000 random inputs, including `n = 0`. Then look at the `-O2` assembly
of the C version: what did the compiler generate?
→ `solutions/ex01_rotate.c`

### Ex 2 — memset and count_char in a `.S` file ⭐⭐
- `void* asm_memset(void* dst, int c, size_t n)` returns `dst`. (x86: try `rep stosb`.)
- `size_t asm_count_char(const char* s, int c)` counts occurrences of `c`
  **without branches inside the loop body**, apart from the loop condition itself
  (x86: `sete`, AArch64: `cinc`).
→ `solutions/ex02_mem.c` + `solutions/ex02_mem.S`

### Ex 3 — Iteration and recursion ⭐⭐⭐
- `uint64_t asm_fib(unsigned n)`: iterative.
- `uint64_t asm_factorial(unsigned n)`: **recursive**. It calls itself, so it must
  build a stack frame, save a callee-saved register and keep the stack aligned.
  Single-step it in lldb/gdb and watch the stack grow.
→ `solutions/ex03_recursion.c` + `solutions/ex03_recursion.S`

### Ex 4 — Assembly that calls C in a loop ⭐⭐⭐
`void asm_apply(int64_t* arr, size_t n, int64_t (*fn)(int64_t))` replaces each
element with `fn(element)`. `arr`, `n` and `fn` must survive every call, so
keep them in callee-saved registers. Test it with a callback that calls
`printf` (which trashes every caller-saved register) to prove your saves work.
→ `solutions/ex04_apply.c` + `solutions/ex04_apply.S`

### Ex 5 — A micro-benchmark harness ⭐⭐
Using the tick counter from `examples/02`, benchmark four popcount
implementations over 1M values: a bit-by-bit loop, Kernighan's `x &= x - 1`,
`__builtin_popcountll`, and an inline-asm version (x86 `popcntq`; AArch64
`cnt` + `addv` on a vector register). Report the **minimum** ticks per call
over several runs (the minimum is the least noisy statistic), and use the
results so the compiler can't delete the work. If a result comes out as ~0
ticks, the optimizer moved the work outside your timed region. Find out how to
stop it (hint: empty `asm volatile` statements with the right operands and clobbers).
→ `solutions/ex05_popcount_bench.c`
