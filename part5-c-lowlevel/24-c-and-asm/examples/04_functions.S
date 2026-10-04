// 04_functions.S: functions written in assembly, called from 04_calling_asm.c.
// One file, two architectures: the preprocessor keeps only the matching half.
#include "../include/asm_macros.h"

    .text

#if defined(__x86_64__)
// ============================ x86-64, System V ABI ============================
// args: rdi, rsi, rdx, rcx, r8, r9   return: rax
// callee-saved (preserve if used): rbx, rbp, r12-r15     everything else is scratch

// int64_t asm_add(int64_t a, int64_t b)
FUNC_BEGIN asm_add
    leaq    (%rdi,%rsi), %rax       // rax = rdi + rsi (lea computes an address, here used as math)
    ret
FUNC_END asm_add

// int64_t asm_sum_array(const int64_t* a, size_t n)
FUNC_BEGIN asm_sum_array
    xorl    %eax, %eax              // sum = 0 (writing eax also zeroes the top half of rax)
    testq   %rsi, %rsi
    jz      2f                      // n == 0 → done   ("2f" = label 2, searching forward)
1:  addq    (%rdi), %rax            // sum += *a
    addq    $8, %rdi                // a++  (8 bytes per int64_t)
    decq    %rsi
    jnz     1b                      // loop while --n != 0
2:  ret
FUNC_END asm_sum_array

// size_t asm_strlen(const char* s)
FUNC_BEGIN asm_strlen
    movq    %rdi, %rax              // p = s
1:  cmpb    $0, (%rax)              // *p == 0 ?
    je      2f
    incq    %rax
    jmp     1b
2:  subq    %rdi, %rax              // return p - s
    ret
FUNC_END asm_strlen

// int64_t asm_max3(int64_t a, int64_t b, int64_t c): branch-free, using conditional moves
FUNC_BEGIN asm_max3
    movq    %rdi, %rax
    cmpq    %rsi, %rax
    cmovlq  %rsi, %rax              // if (rax < b) rax = b
    cmpq    %rdx, %rax
    cmovlq  %rdx, %rax              // if (rax < c) rax = c
    ret
FUNC_END asm_max3

// int64_t asm_call_twice(int64_t (*fn)(int64_t), int64_t x): returns fn(fn(x))
// Calls back into C, so it must follow the rules a C function would:
//  - keep fn somewhere the callee won't destroy → a callee-saved register (rbx), saved first
//  - the stack must be 16-byte aligned at each `call`. On entry rsp ≡ 8 (mod 16), because the
//    return address was just pushed; one more push makes it ≡ 0 again.
FUNC_BEGIN asm_call_twice
    pushq   %rbx
    movq    %rdi, %rbx              // rbx = fn
    movq    %rsi, %rdi              // first argument = x
    callq   *%rbx                   // rax = fn(x)
    movq    %rax, %rdi
    callq   *%rbx                   // rax = fn(fn(x))
    popq    %rbx
    ret
FUNC_END asm_call_twice

#elif defined(__aarch64__)
// ============================ AArch64, AAPCS64 ABI ============================
// args: x0-x7   return: x0   link register (return address): x30   frame pointer: x29
// callee-saved: x19-x28, x29, x30 (if you make calls), sp      scratch: x0-x18 (x18 reserved on Apple)

FUNC_BEGIN asm_add
    add     x0, x0, x1
    ret
FUNC_END asm_add

FUNC_BEGIN asm_sum_array
    mov     x2, #0                  // sum
    cbz     x1, 2f                  // n == 0 → done
1:  ldr     x3, [x0], #8            // x3 = *a; a += 8 (post-increment addressing)
    add     x2, x2, x3
    subs    x1, x1, #1              // n--, setting flags
    b.ne    1b
2:  mov     x0, x2
    ret
FUNC_END asm_sum_array

FUNC_BEGIN asm_strlen
    mov     x1, x0
1:  ldrb    w2, [x1], #1            // load a byte, then advance
    cbnz    w2, 1b
    sub     x0, x1, x0
    sub     x0, x0, #1              // we advanced one past the terminator
    ret
FUNC_END asm_strlen

FUNC_BEGIN asm_max3
    cmp     x0, x1
    csel    x0, x0, x1, gt          // x0 = (x0 > x1) ? x0 : x1
    cmp     x0, x2
    csel    x0, x0, x2, gt
    ret
FUNC_END asm_max3

// `bl`/`blr` overwrite x30 (our return address), so save it, plus x19 to hold fn.
// sp must stay 16-byte aligned: save in 16-byte units.
FUNC_BEGIN asm_call_twice
    stp     x29, x30, [sp, #-32]!   // push frame pointer + link register; sp -= 32
    mov     x29, sp
    str     x19, [sp, #16]
    mov     x19, x0                 // x19 = fn
    mov     x0, x1                  // x0 = x
    blr     x19                     // x0 = fn(x)
    blr     x19                     // x0 = fn(fn(x))
    ldr     x19, [sp, #16]
    ldp     x29, x30, [sp], #32     // pop; sp += 32
    ret
FUNC_END asm_call_twice

#else
#error "04_functions.S supports x86-64 and AArch64 only"
#endif

NOTE_GNU_STACK
