# Part 5 — C and Low-Level Programming

C++ grew out of C, and C is still the language of operating systems,
firmware, device drivers and every "foreign function interface" (Python,
Rust, Go and C++ all talk to each other via C). This part takes what you
learned in Part 1 and removes the safety net: no destructors, no
`std::vector`, no exceptions. You manage every byte yourself, then go one level
lower still, to hardware registers and assembly.

| # | Chapter | You will learn |
|---|---------|----------------|
| 21 | [C Essentials for C++ Programmers](21-c-essentials/README.md) | what C lacks, structs, strings, the preprocessor, opaque types, error handling with `goto` |
| 22 | [Memory Management in C](22-c-memory/README.md) | `malloc`/`calloc`/`realloc`/`free`, ownership conventions, dynamic arrays, arenas, pools, alignment, ASan |
| 23 | [Writing a Hardware Controller](23-hardware-controller/README.md) | memory-mapped I/O, `volatile`, bit manipulation, register maps, drivers, interrupts, ring buffers, a simulated fan controller + a real Cortex-M port |
| 24 | [Combining C and Assembly](24-c-and-asm/README.md) | reading compiler output, calling conventions (x86-64 & AArch64), extended inline asm, separate `.S` files, calling C from asm |

## Building

Every `.c` file in `examples/` and `solutions/` is a standalone program, as
in Part 1. Chapters 23 and 24 contain multi-file programs, so they have their
own `CMakeLists.txt`.

```bash
# Without CMake: compile (and run) everything in part 5
./scripts/check.sh --run part5

# By hand
cc -std=c17 -Wall -Wextra -Wpedantic part5-c-lowlevel/22-c-memory/examples/01_malloc_family.c -o m && ./m

# With CMake (from the repo root), like the other parts
cmake -S . -B build && cmake --build build -j
./build/part5-c-lowlevel/22-c-memory/ch22_ex_01_malloc_family
```

All C code targets **C17** and compiles cleanly with `-Wall -Wextra -Wpedantic`
under clang and GCC. Chapter 24 has assembly for **x86-64** and **AArch64
(Apple Silicon, Raspberry Pi 4/5, AWS Graviton)**, and it builds on macOS and
Linux.

## Recommended order

Start with 21 even if you know C++ well: the differences bite. Chapter 22 is
the core of this part. 23 and 24 are independent of each other, but both
assume 22.
