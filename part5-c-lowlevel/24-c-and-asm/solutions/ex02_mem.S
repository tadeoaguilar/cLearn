#include "../include/asm_macros.h"
    .text

#if defined(__x86_64__)

// void* asm_memset(void* dst /*rdi*/, int c /*esi*/, size_t n /*rdx*/)
FUNC_BEGIN asm_memset
    movq    %rdi, %r8               // remember dst for the return value (stosb advances rdi)
    movl    %esi, %eax              // al = byte to store
    movq    %rdx, %rcx              // rcx = count
    rep stosb                       // while (rcx--) *rdi++ = al;  (the ABI guarantees the direction flag is clear)
    movq    %r8, %rax
    ret
FUNC_END asm_memset

// size_t asm_count_char(const char* s /*rdi*/, int c /*esi*/)
FUNC_BEGIN asm_count_char
    xorl    %eax, %eax              // count = 0
1:  movzbl  (%rdi), %edx            // load a byte, zero-extended
    testl   %edx, %edx
    jz      2f                      // end of string
    xorl    %ecx, %ecx
    cmpb    %sil, %dl               // compare with the low byte of c
    sete    %cl                     // cl = (byte == c), no branch
    addq    %rcx, %rax
    incq    %rdi
    jmp     1b
2:  ret
FUNC_END asm_count_char

#elif defined(__aarch64__)

// void* asm_memset(void* dst /*x0*/, int c /*w1*/, size_t n /*x2*/): x0 is already the return value
FUNC_BEGIN asm_memset
    mov     x3, x0
    cbz     x2, 2f
1:  strb    w1, [x3], #1
    subs    x2, x2, #1
    b.ne    1b
2:  ret
FUNC_END asm_memset

// size_t asm_count_char(const char* s /*x0*/, int c /*w1*/)
FUNC_BEGIN asm_count_char
    mov     x2, x0
    mov     x0, #0
    and     w1, w1, #0xff           // compare as unsigned char
1:  ldrb    w3, [x2], #1
    cbz     w3, 2f
    cmp     w3, w1
    cinc    x0, x0, eq              // x0 += (byte == c), no branch
    b       1b
2:  ret
FUNC_END asm_count_char

#else
#error "x86-64 or AArch64 only"
#endif

NOTE_GNU_STACK
