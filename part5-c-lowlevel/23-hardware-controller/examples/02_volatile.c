// Why `volatile` exists: a register that changes behind the compiler's back.
//
// A second thread plays the role of hardware. After a delay it sets a READY bit in a
// "status register". The main thread polls that bit, the way a driver polls hardware.
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

typedef struct {
    volatile uint32_t STATUS; // try deleting `volatile` and compiling with -O2: the
    volatile uint32_t DATA;   // poll loop can turn into `if (!ready) for (;;);` and hang
} FakeDevice;

static FakeDevice device; // on real hardware: #define DEVICE ((FakeDevice*)0x40010000u)
#define STATUS_READY (1u << 0)

static void* hardware(void* arg) {
    (void)arg;
    struct timespec ts = {0, 50 * 1000 * 1000}; // 50 ms "conversion time"
    nanosleep(&ts, NULL);
    device.DATA = 0xBEEF;
    device.STATUS |= STATUS_READY;
    return NULL;
}

int main(void) {
    pthread_t hw;
    pthread_create(&hw, NULL, hardware, NULL);

    unsigned long polls = 0;
    while (!(device.STATUS & STATUS_READY)) polls++; // each iteration really reads memory

    printf("device ready after %lu polls, DATA = 0x%X\n", polls, (unsigned)device.DATA);
    pthread_join(hw, NULL);

    // What volatile does NOT do:
    //  - it does not make `x++` atomic (that's still read, add, write)
    //  - it does not order other non-volatile memory accesses or add CPU memory barriers
    //  - it is not a substitute for _Atomic or mutexes between threads in portable code
    // Use volatile for memory-mapped I/O and for variables shared with interrupt handlers or
    // signal handlers (volatile sig_atomic_t). Use <stdatomic.h> for thread synchronization.
    printf("see the comments for what volatile does and does not guarantee\n");
    return 0;
}
