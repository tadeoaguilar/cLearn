# Glossary

- **ABI**: Application Binary Interface. How compiled code agrees on calling
  conventions and object layout. Mixing compilers or standard libraries can break it.
- **Aggregate**: a simple struct or array with no user constructors, which you
  can brace-initialize: `Point p{1, 2};`.
- **Compilation unit / translation unit (TU)**: one `.cpp` file after all its
  `#include`s are pasted in. Each TU compiles separately.
- **Concept** (C++20): a named compile-time requirement on template parameters.
- **const-correctness**: marking everything you don't modify as `const`, so the
  compiler enforces it.
- **Dangling reference/pointer**: refers to an object that has been destroyed.
  Using it is UB.
- **Declaration vs definition**: a declaration says something exists
  (`int f();`). A definition provides it (`int f() { return 1; }`).
- **Header file (`.h`/`.hpp`)**: declarations shared between TUs via `#include`.
- **Iterator**: a generalized pointer used by the STL to walk containers.
- **Linker**: combines object files and libraries into an executable and
  resolves symbols. "Undefined reference" errors come from here.
- **lvalue / rvalue**: an lvalue has an identity (a name or address). An rvalue
  is a temporary. Move semantics is built on rvalues.
- **Move**: transferring resources from one object to another instead of copying.
- **ODR**: One Definition Rule. Each function/variable has exactly one definition
  across the program (with exceptions for `inline` and templates).
- **RAII**: Resource Acquisition Is Initialization. Constructors acquire,
  destructors release. This is the core idiom of C++.
- **Repository pattern**: an interface that hides how data is stored (memory,
  files, a database) from business logic.
- **Smart pointer**: an object that owns a heap allocation and frees it
  automatically (`unique_ptr`, `shared_ptr`).
- **STL**: Standard Template Library. The containers, iterators and algorithms of the standard library.
- **Template**: a blueprint the compiler uses to generate code for specific types.
- **UB (Undefined Behavior)**: code the standard gives no meaning to (out-of-bounds
  access, signed overflow, use-after-free…). Anything can happen.
- **UObject / UCLASS / UPROPERTY** (Unreal): Unreal's reflection system that
  enables garbage collection, Blueprints, serialization and the editor.
- **vtable**: the hidden table of function pointers that makes `virtual` calls work.

## Part 5 (C and low level)

- **Arena allocator**: hands out memory by bumping an offset in one big block
  and frees everything at once. Ideal for per-frame or per-request data.
- **Calling convention**: the part of the ABI that says which registers hold
  arguments and return values, and which registers a function must preserve.
- **Callee-saved register**: a register a function must restore before it
  returns (x86-64: `rbx`, `rbp`, `r12`–`r15`; AArch64: `x19`–`x28`).
- **Critical section**: code that runs with interrupts (or other threads)
  locked out, so shared multi-word data stays consistent.
- **Inline assembly**: assembly instructions embedded in C with
  `__asm__("..." : outputs : inputs : clobbers)`.
- **Interrupt service routine (ISR)**: a function the CPU runs when hardware
  signals an event. It must be short and must acknowledge the event.
- **Memory-mapped I/O (MMIO)**: controlling hardware by reading and writing
  special addresses that are wired to device registers.
- **Opaque type**: a struct declared in a header but defined only in one `.c`
  file, so users can hold pointers to it but not see inside. C's `private`.
- **Pool allocator**: hands out fixed-size blocks from a free list in O(1),
  with no fragmentation.
- **Vector table**: an array of handler addresses that the CPU consults on
  reset and on each interrupt.
- **`volatile`**: tells the compiler that every access to a variable is an
  observable side effect, so it must not be cached, removed or reordered.
- **W1C (write-1-to-clear)**: a register type where writing a 1 to a bit
  clears that flag.
