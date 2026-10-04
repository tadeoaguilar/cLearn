#include "../include/asm_macros.h"
    .text

#if defined(__x86_64__)

// uint64_t asm_fib(unsigned n /*edi*/)
FUNC_BEGIN asm_fib
    xorl    %eax, %eax              // a = 0
    movl    $1, %edx                // b = 1
    movl    %edi, %ecx
    testl   %ecx, %ecx
    jz      2f
1:  leaq    (%rax,%rdx), %r8        // t = a + b
    movq    %rdx, %rax              // a = b
    movq    %r8, %rdx               // b = t
    decl    %ecx
    jnz     1b
2:  ret
FUNC_END asm_fib

// uint64_t asm_factorial(unsigned n /*edi*/): n <= 1 ? 1 : n * asm_factorial(n - 1)
FUNC_BEGIN asm_factorial
    cmpl    $1, %edi
    ja      1f
    movl    $1, %eax                // base case: no frame needed (a "leaf" path)
    ret
1:  pushq   %rbx                    // save the caller's rbx; this also realigns rsp to 16
    movl    %edi, %ebx              // keep n across the call (writing ebx zero-extends into rbx)
    leal    -1(%rdi), %edi          // argument: n - 1
    call    SYM(asm_factorial)
    imulq   %rbx, %rax              // rax = n * factorial(n - 1)
    popq    %rbx
    ret
FUNC_END asm_factorial

#elif defined(__aarch64__)

FUNC_BEGIN asm_fib
    mov     x1, #0                  // a
    mov     x2, #1                  // b
    cbz     w0, 2f
1:  add     x3, x1, x2
    mov     x1, x2
    mov     x2, x3
    subs    w0, w0, #1
    b.ne    1b
2:  mov     x0, x1
    ret
FUNC_END asm_fib

FUNC_BEGIN asm_factorial
    cmp     w0, #1
    b.hi    1f
    mov     x0, #1
    ret
1:  stp     x29, x30, [sp, #-32]!   // save fp and lr (bl will overwrite lr); 32 keeps sp 16-aligned
    mov     x29, sp
    str     x19, [sp, #16]          // save the caller's x19
    mov     w19, w0                 // keep n across the call
    sub     w0, w0, #1
    bl      SYM(asm_factorial)
    mul     x0, x0, x19
    ldr     x19, [sp, #16]
    ldp     x29, x30, [sp], #32
    ret
FUNC_END asm_factorial

#else
#error "x86-64 or AArch64 only"
#endif

NOTE_GNU_STACK
