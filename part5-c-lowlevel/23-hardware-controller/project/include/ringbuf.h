// ringbuf.h: a single-producer / single-consumer byte queue.
//
// The producer is an interrupt handler and the consumer is the main loop.
// Neither side ever writes the other's index, so no lock is needed. The only
// requirement is that index updates become visible in the right order, which C11
// atomics with acquire/release ordering guarantee on every CPU.
//
//   buf: [ . . a b c d . . ]     capacity must be a power of two:
//              ↑tail   ↑head     index & (CAP - 1) replaces the slower index % CAP
#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#define RINGBUF_CAP 64u
_Static_assert((RINGBUF_CAP & (RINGBUF_CAP - 1)) == 0, "capacity must be a power of two");

typedef struct {
    uint8_t data[RINGBUF_CAP];
    _Atomic uint32_t head; // written only by the producer
    _Atomic uint32_t tail; // written only by the consumer
    uint32_t dropped;      // producer-side statistic
} RingBuf;

// head and tail are free-running counters. Unsigned wrap-around makes (head - tail) the fill level.
static inline bool ringbuf_push(RingBuf* rb, uint8_t byte) { // producer only
    uint32_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    uint32_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    if (head - tail == RINGBUF_CAP) {
        rb->dropped++;
        return false; // full
    }
    rb->data[head & (RINGBUF_CAP - 1)] = byte;
    atomic_store_explicit(&rb->head, head + 1, memory_order_release); // publish the byte AFTER writing it
    return true;
}

static inline bool ringbuf_pop(RingBuf* rb, uint8_t* out) { // consumer only
    uint32_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);
    uint32_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    if (head == tail) return false; // empty
    *out = rb->data[tail & (RINGBUF_CAP - 1)];
    atomic_store_explicit(&rb->tail, tail + 1, memory_order_release); // free the slot AFTER reading it
    return true;
}

#endif
