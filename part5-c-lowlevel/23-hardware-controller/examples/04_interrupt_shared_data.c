// Sharing multi-word data with an interrupt handler: torn reads and how to avoid them.
//
// An "ISR" (a thread here, ticking as fast as it can) keeps a clock of seconds +
// milliseconds. If the main code reads `sec` and then `ms`, and the ISR rolls 999 → 0
// between the two reads, main sees the OLD second with the NEW millisecond: a time
// almost 1 s in the past. Each word is read correctly, but the pair is inconsistent.
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    volatile uint32_t sec;
    volatile uint32_t ms;
} Clock;

static Clock clk;
static atomic_bool stop;
static pthread_mutex_t irq_mask = PTHREAD_MUTEX_INITIALIZER; // stands in for "disable interrupts"

static void* tick_isr(void* arg) {
    (void)arg;
    while (!atomic_load(&stop)) {
        pthread_mutex_lock(&irq_mask); // the ISR can't run while main has interrupts disabled
        if (++clk.ms == 1000) {
            clk.ms = 0;
            clk.sec++;
        }
        pthread_mutex_unlock(&irq_mask);
    }
    return NULL;
}

static uint64_t read_naive(void) { return (uint64_t)clk.sec * 1000 + clk.ms; }

static uint64_t read_critical(void) { // option 1: disable interrupts around the read
    pthread_mutex_lock(&irq_mask);    // on Cortex-M: __disable_irq() / cpsid i
    uint64_t t = (uint64_t)clk.sec * 1000 + clk.ms;
    pthread_mutex_unlock(&irq_mask);  // keep critical sections SHORT: they delay interrupts
    return t;
}

static uint64_t read_retry(void) { // option 2: read until two consecutive reads agree (no masking)
    uint32_t s1, ms, s2;
    do {
        s1 = clk.sec;
        ms = clk.ms;
        s2 = clk.sec;
    } while (s1 != s2);
    return (uint64_t)s1 * 1000 + ms;
}

static unsigned count_glitches(uint64_t (*reader)(void), int samples) {
    unsigned glitches = 0;
    uint64_t prev = reader();
    for (int i = 0; i < samples; i++) {
        uint64_t now = reader();
        if (now < prev) glitches++; // a clock must never go backwards
        prev = now;
    }
    return glitches;
}

int main(void) {
    pthread_t t;
    pthread_create(&t, NULL, tick_isr, NULL);
    enum { N = 2000000 };
    printf("naive read:      %u glitches in %d samples\n", count_glitches(read_naive, N), N);
    printf("critical section: %u glitches\n", count_glitches(read_critical, N));
    printf("retry loop:       %u glitches\n", count_glitches(read_retry, N));
    printf("(the naive count depends on timing; it can be 0 on a lucky run. That's what makes these bugs nasty.)\n");
    atomic_store(&stop, true);
    pthread_join(t, NULL);
    return 0;
}
