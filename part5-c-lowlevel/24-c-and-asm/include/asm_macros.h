// asm_macros.h: lets one .S file assemble on macOS (Mach-O) and Linux (ELF).
//
// .S files (capital S) go through the C preprocessor before the assembler, so
// #if, #include and // comments all work in them. The function wrappers are
// ASSEMBLER macros (.macro), not C macros. A C macro can't expand to several lines,
// and the usual workaround of separating statements with ';' breaks on Apple
// arm64, where ';' starts a comment.
//
// Differences handled here:
//   - macOS prefixes C symbol names with an underscore: C's `asm_add` is `_asm_add`
//   - ELF wants .type/.size so debuggers and profilers know where functions end
//   - Linux wants a note saying "this object doesn't need an executable stack"
//
// Usage:
//   FUNC_BEGIN my_function
//       ...instructions...
//   FUNC_END my_function
//
// To CALL a C-visible symbol from asm, use SYM(): `call SYM(puts)` / `bl SYM(puts)`.
#ifndef ASM_MACROS_H
#define ASM_MACROS_H

#if defined(__APPLE__)
#define SYM(name) _##name
#else
#define SYM(name) name
#endif

#if defined(__APPLE__)

.macro FUNC_BEGIN name
    .globl _\name
    .p2align 4
_\name:
.endm

.macro FUNC_END name
.endm

.macro NOTE_GNU_STACK
.endm

#else

.macro FUNC_BEGIN name
    .globl \name
    .type \name, %function
    .p2align 4
\name:
.endm

.macro FUNC_END name
    .size \name, . - \name
.endm

.macro NOTE_GNU_STACK
    .section .note.GNU-stack, "", %progbits
.endm

#endif
#endif
