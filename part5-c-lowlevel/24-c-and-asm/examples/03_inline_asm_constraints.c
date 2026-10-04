// Inline asm, part 2: multiple outputs, flags, clobbers and a spinlock.
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// 1. Overflow detection: read the CPU's overflow flag right after the add.
static bool add_overflows(int32_t a, int32_t b, int32_t* sum) {
    uint8_t of;
#if defined(__x86_64__)
    __asm__("addl %[b], %[a]\n\t"
            "seto %[of]"                   // of = overflow flag
            : [a] "+r"(a), [of] "=q"(of)   // "+r": a is read AND written; "q": a byte-addressable register
            : [b] "r"(b)
            : "cc");
#elif defined(__aarch64__)
    uint32_t flag;
    __asm__("adds %w[a], %w[a], %w[b]\n\t" // adds = add and set flags
            "cset %w[f], vs"               // f = 1 if the V (overflow) flag is set
            : [a] "+r"(a), [f] "=r"(flag)
            : [b] "r"(b)
            : "cc");
    of = (uint8_t)flag;
#else
    of = (uint8_t)__builtin_add_overflow(a, b, &a);
#endif
    *sum = a;
    return of;
}

// 2. Two outputs: the full 128-bit product of two 64-bit numbers.
static void mul_64x64(uint64_t x, uint64_t y, uint64_t* hi, uint64_t* lo) {
#if defined(__x86_64__)
    __asm__("mulq %3" : "=a"(*lo), "=d"(*hi) : "a"(x), "rm"(y) : "cc"); // rdx:rax = rax * operand
#elif defined(__aarch64__)
    __asm__("mul %0, %2, %3\n\t"
            "umulh %1, %2, %3" // the high 64 bits of the unsigned product
            : "=&r"(*lo), "=r"(*hi) // "&" (early clobber): lo is written before x and y are last read
            : "r"(x), "r"(y));
#else
    unsigned __int128 p = (unsigned __int128)x * y;
    *lo = (uint64_t)p, *hi = (uint64_t)(p >> 64);
#endif
}

// 3. Atomic exchange → a spinlock. The "memory" clobber is essential: it tells the compiler
//    that memory may have changed and must not be cached in registers across the asm.
static uint64_t atomic_swap(volatile uint64_t* p, uint64_t v) {
#if defined(__x86_64__)
    __asm__ volatile("xchgq %0, %1" : "+r"(v), "+m"(*p) : : "memory"); // xchg with memory is always locked
    return v;
#elif defined(__aarch64__)
    uint64_t old;
    uint32_t failed;
    __asm__ volatile("1: ldaxr %0, [%2]\n\t"     // load-acquire exclusive
                     "   stlxr %w1, %3, [%2]\n\t" // store-release exclusive; failed=1 if someone interfered
                     "   cbnz %w1, 1b"            // retry
                     : "=&r"(old), "=&r"(failed)
                     : "r"(p), "r"(v)
                     : "memory");
    return old;
#else
    return __atomic_exchange_n(p, v, __ATOMIC_SEQ_CST);
#endif
}

static volatile uint64_t lock_word;
static long counter;

static void spin_lock(void) {
    while (atomic_swap(&lock_word, 1) != 0) {
        // spin. Real code would add a pause/yield hint here (exercise!)
    }
}
static void spin_unlock(void) {
    __asm__ volatile("" ::: "memory"); // compiler barrier: keep the critical section above this line
    lock_word = 0;                     // fine on x86 (TSO). On ARM a real lock needs a store-release (stlr).
}

static void* worker(void* arg) {
    (void)arg;
    for (int i = 0; i < 200000; i++) {
        spin_lock();
        counter++;
        spin_unlock();
    }
    return NULL;
}

int main(void) {
    int32_t s;
    bool of = add_overflows(2000000000, 2000000000, &s);
    printf("2e9 + 2e9 → %d, overflow=%s\n", s, of ? "yes" : "no");
    of = add_overflows(-5, 3, &s);
    printf("-5 + 3    → %d, overflow=%s\n", s, of ? "yes" : "no");

    uint64_t hi, lo;
    mul_64x64(0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull, &hi, &lo);
    printf("(2^64-1)^2 = 0x%016llx_%016llx\n", (unsigned long long)hi, (unsigned long long)lo);

    pthread_t t[4];
    for (int i = 0; i < 4; i++) pthread_create(&t[i], NULL, worker, NULL);
    for (int i = 0; i < 4; i++) pthread_join(t[i], NULL);
    printf("4 threads × 200000 locked increments = %ld (expected 800000)\n", counter);
    return 0;
}
