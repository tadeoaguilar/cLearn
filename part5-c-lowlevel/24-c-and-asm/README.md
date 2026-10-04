# 24 — Combining C and Assembly

> Goal: read the assembly your compiler produces, understand the calling
> convention that lets C and assembly call each other, and write both inline
> assembly and standalone `.S` files for **x86-64** and **AArch64**.

You will rarely *need* assembly. Compilers are excellent, and intrinsics cover
SIMD. But you need it to:
- understand performance, crashes and debugger output ("why is this slow?", "what is at `rip`?")
- use instructions that C can't express: cycle counters, `cpuid`, system registers,
  interrupt masking (`cpsid i` in chapter 23), memory barriers, context switches
- write startup code, bootloaders, OS kernels, and hand-tuned inner loops (crypto, codecs)

## 1. Which CPU am I on?

```bash
uname -m        # x86_64 → Intel/AMD;  arm64 / aarch64 → Apple Silicon, Raspberry Pi, Graviton
```

Every example in this chapter has both versions, selected with
`#if defined(__x86_64__)` / `#elif defined(__aarch64__)`.

## 2. Reading compiler output

```bash
cc -O2 -S -fno-asynchronous-unwind-tables examples/01_see_the_assembly.c -o - | less
cc -O2 -S -masm=intel ...            # x86: Intel syntax instead of AT&T
objdump -d a.out | less              # disassemble a compiled binary
```

Even better, use **[Compiler Explorer](https://godbolt.org)**: paste C on the left
and see the assembly on the right, colour-matched line by line.

### Registers

| x86-64 | AArch64 | Role |
|--------|---------|------|
| `rax`…`rdx`, `rsi`, `rdi`, `r8`–`r15` (16 × 64-bit) | `x0`–`x30` (31 × 64-bit) | general purpose |
| `eax` = low 32 bits of `rax`, `al` = low 8 | `w0` = low 32 bits of `x0` | narrower views |
| `rsp` | `sp` | stack pointer |
| `rbp` (optional) | `x29` | frame pointer |
| (return address is pushed on the stack by `call`) | `x30` = `lr` (link register, set by `bl`) | return address |
| `rip` | `pc` | instruction pointer |
| `rflags` (ZF, CF, OF, SF...) | `nzcv` (N, Z, C, V) | condition flags |
| `xmm0`–`xmm15` (128-bit SIMD) | `v0`–`v31` (128-bit SIMD/FP) | vector and floating point |

### AT&T vs Intel syntax (x86)

| | AT&T (GCC/Clang default) | Intel |
|-|---------------------------|-------|
| operand order | `op src, dst` | `op dst, src` |
| registers / immediates | `%rax`, `$5` | `rax`, `5` |
| size | suffix: `movq`, `movl`, `movb` | implied, or `QWORD PTR` |
| memory | `8(%rdi,%rcx,4)` | `[rdi + rcx*4 + 8]` |

AArch64 has only one syntax: `op dst, src1, src2`, e.g. `add x0, x1, x2`.

## 3. Calling conventions (ABIs)

The **ABI** is the contract that lets separately compiled code (C, C++, Rust,
assembly) call each other. It says where arguments go, where the result comes
back, and which registers a function must leave untouched.

| | x86-64 System V (Linux, macOS) | AArch64 AAPCS64 (Linux, macOS) |
|-|-------------------------------|---------------------------------|
| integer/pointer args | `rdi, rsi, rdx, rcx, r8, r9`, then the stack | `x0`–`x7`, then the stack |
| float args | `xmm0`–`xmm7` | `v0`–`v7` |
| return value | `rax` (+ `rdx` for 128-bit/small structs) | `x0` (+ `x1`) |
| **callee-saved** (you must restore them) | `rbx, rbp, r12–r15` | `x19–x28, x29 (fp), x30 (lr)` |
| caller-saved (scratch, a call may destroy them) | `rax, rcx, rdx, rsi, rdi, r8–r11` | `x0–x18` (Apple reserves `x18`) |
| stack alignment | 16 bytes **at the `call` instruction** | `sp` always 16-byte aligned |
| return | `ret` pops the return address | `ret` jumps to `x30` |

> Windows x64 is different again (`rcx, rdx, r8, r9` + 32 bytes of "shadow
> space"). Part 5's assembly targets Linux and macOS.

A function that calls another function must:
1. save the callee-saved registers it uses (push/stp) and restore them before returning
2. keep anything it needs after the call in callee-saved registers (or on the stack)
3. keep the stack aligned. Misalignment crashes inside `printf` or SSE code, far from the real bug.
4. on AArch64, save `x30` before `bl`, because `bl` overwrites it

## 4. Extended inline assembly (GCC/Clang)

```c
__asm__ [volatile] ("template" : outputs : inputs : clobbers);

int sum;
__asm__("addl %2, %0" : "=r"(sum) : "0"(a), "r"(b) : "cc");   // x86: sum = a + b
__asm__("add %w0, %w1, %w2" : "=r"(sum) : "r"(a), "r"(b));      // AArch64
```

| Piece | Meaning |
|-------|---------|
| `"r"` / `"m"` / `"i"` | operand in a register / in memory / an immediate |
| `"=r"` | write-only output. `"+r"`: read and written |
| `"=&r"` | **early clobber**: written before all inputs are consumed, so don't share a register with an input |
| `"0"` | the same location as operand 0 |
| `"a"`, `"d"`, `"c"`... | x86: a specific register (eax, edx, ecx) |
| `%[name]` | named operands: `[sum] "=r"(sum)` |
| `"cc"` clobber | the asm changes the flags |
| `"memory"` clobber | the asm reads/writes memory the compiler can't see → don't cache values across it |
| `volatile` | has side effects; don't delete it, hoist it out of loops, or merge it with an identical one |

`__asm__ volatile("" ::: "memory")` emits **no instruction**. It is a
*compiler barrier* that stops reordering of memory accesses across it.

**The compiler does not understand your template.** It only trusts the
constraints. A missing clobber or a wrong constraint is a bug that may appear
only at `-O2`, in a different function, months later. That makes intrinsics
(section 6) the better choice when one exists.

## 5. Separate assembly files (`.S`)

For anything longer than a few instructions, write a real assembly file. Use a
capital **`.S`** extension, which means "run the C preprocessor first", so you
get `#include`, `#if` and `//` comments. Add it to the build like a `.c` file:

```bash
cc -std=c17 main.c functions.S -o program
```

```asm
#include "../include/asm_macros.h"   // hides macOS's leading underscore, ELF .type/.size
    .text
FUNC_BEGIN asm_add                   // becomes:  .globl _asm_add / _asm_add:  on macOS
    leaq    (%rdi,%rsi), %rax
    ret
FUNC_END asm_add
```

On the C side, declare a prototype: `int64_t asm_add(int64_t, int64_t);`.
The prototype is an **unchecked promise** that the assembly follows the ABI.
From C++ it would need `extern "C"`, because C++ mangles names (chapter 21 §8).

## 6. Intrinsics: usually better than asm

```c
#include <immintrin.h>   // x86: SSE, AVX
__m128 v = _mm_add_ps(_mm_loadu_ps(a), _mm_loadu_ps(b));   // 4 float additions, 1 instruction
#include <arm_neon.h>    // AArch64: NEON
float32x4_t w = vaddq_f32(vld1q_f32(a), vld1q_f32(b));
```

Intrinsics give you specific instructions while the compiler keeps doing
register allocation, scheduling and inlining. `__builtin_popcount`,
`__builtin_ctz`, `__builtin_bswap32` and `__builtin_expect` are portable
built-ins that compile to the best instruction for the target.

## 7. Debugging at the instruction level

```bash
cc -g -O0 examples/04_calling_asm.c examples/04_functions.S -o calling_asm
lldb ./calling_asm            # or gdb
(lldb) b asm_call_twice
(lldb) run
(lldb) disassemble            # where am I?
(lldb) register read rdi rsi  # (x0 x1 on ARM)
(lldb) si                     # step ONE instruction
(lldb) memory read -s8 -fx -c4 $sp   # look at the stack
```

## Examples

| File | Shows |
|------|-------|
| `01_see_the_assembly.c` | functions worth reading with `-S`: lea, magic-number division, cmov, jump tables, closed-form loops |
| `02_inline_asm_basics.c` | a first `asm` statement, `rdtsc`/`cntvct_el0`, `cpuid`/`cntfrq_el0` |
| `03_inline_asm_constraints.c` | flags as outputs, two outputs (128-bit multiply), early clobber, a spinlock built from atomic exchange |
| `04_calling_asm.c` + `04_functions.S` | C calling asm (add, loops, strings, cmov/csel), and asm calling back into C |
| `05_simd_intrinsics.c` | SSE / NEON intrinsics vs scalar code, with timing |

```bash
./scripts/check.sh --run 24-c-and-asm        # compiles .c files, plus their .S partners
cc -std=c17 examples/04_calling_asm.c examples/04_functions.S -o calling_asm && ./calling_asm
```

## Pitfalls

- Forgetting to save a callee-saved register. The *caller* breaks later,
  in code that looks perfectly fine.
- Misaligning the stack before a `call` (x86-64: an odd number of 8-byte pushes).
- On AArch64, calling a function without saving `x30`: your `ret` jumps into the callee and loops.
- Writing a 32-bit register: on x86-64 `movl` **zeroes** the upper 32 bits of
  the 64-bit register. `movw`/`movb` don't. On AArch64, writing `w0` zeroes the top of `x0`.
- Missing `"memory"`/`"cc"` clobbers, or a missing `volatile` on asm with side effects.
- Mach-O vs ELF: the leading underscore, `.type`/`.size`, and `;` being a comment
  on Apple arm64. `asm_macros.h` handles all three.

➡️ That's the end of Part 5. From here, try writing a tiny kernel (osdev.org),
an emulator (CHIP-8 is a classic first one), or port chapter 23's fan
controller to a real board.
