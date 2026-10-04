#include "../include/asm_macros.h"
    .text

// void asm_apply(int64_t* arr, size_t n, int64_t (*fn)(int64_t))
#if defined(__x86_64__)

FUNC_BEGIN asm_apply
    pushq   %rbx                    // 3 callee-saved registers. On entry rsp ≡ 8 (mod 16),
    pushq   %r12                    // after 3 pushes ≡ 0, so the stack is aligned for the calls
    pushq   %r13
    movq    %rdi, %rbx              // rbx = arr   (survives calls)
    movq    %rsi, %r12              // r12 = n
    movq    %rdx, %r13              // r13 = fn
    testq   %r12, %r12
    jz      2f
1:  movq    (%rbx), %rdi            // argument = *arr
    call    *%r13                   // may destroy rax, rcx, rdx, rsi, rdi, r8-r11...
    movq    %rax, (%rbx)            // ...but not rbx, r12, r13
    addq    $8, %rbx
    decq    %r12
    jnz     1b
2:  popq    %r13                    // restore in reverse order
    popq    %r12
    popq    %rbx
    ret
FUNC_END asm_apply

#elif defined(__aarch64__)

FUNC_BEGIN asm_apply
    stp     x29, x30, [sp, #-48]!   // frame: fp, lr, x19, x20, x21 (+8 padding) = 48 bytes
    mov     x29, sp
    stp     x19, x20, [sp, #16]
    str     x21, [sp, #32]
    mov     x19, x0                 // arr
    mov     x20, x1                 // n
    mov     x21, x2                 // fn
    cbz     x20, 2f
1:  ldr     x0, [x19]
    blr     x21
    str     x0, [x19], #8           // *arr++ = result
    subs    x20, x20, #1
    b.ne    1b
2:  ldr     x21, [sp, #32]
    ldp     x19, x20, [sp, #16]
    ldp     x29, x30, [sp], #48
    ret
FUNC_END asm_apply

#else
#error "x86-64 or AArch64 only"
#endif

NOTE_GNU_STACK
