#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

enum { DEBOUNCE_MAX = 10 }; // 10 consistent ms

typedef struct {
    uint8_t integrator;
    bool state; // debounced: true = pressed
} Debouncer;

// Called every 1 ms. Returns true exactly once per press (on the debounced rising edge).
static bool debounce(Debouncer* d, bool raw_pressed) {
    if (raw_pressed) {
        if (d->integrator < DEBOUNCE_MAX) d->integrator++;
    } else if (d->integrator > 0) {
        d->integrator--;
    }
    bool old = d->state;
    if (d->integrator == DEBOUNCE_MAX) d->state = true;
    else if (d->integrator == 0) d->state = false;
    return d->state && !old;
}

// A synthetic signal described by the times (ms) at which the raw level toggles.
// Press 1 bounces on the way down and on the way up; a 1 ms glitch hits while it's
// held and another while it's released; press 2 bounces briefly.
static const int toggles[] = {
    100, 102, 103, 107, 108,      // press 1: contact bounce, settles pressed at 108
    200, 201,                     // 1 ms glitch while pressed
    300, 301, 304, 306, 309,      // release: bounce, settles released at 309
    400, 401,                     // 1 ms glitch while released (EMI)
    600, 601, 602, 603, 604,      // press 2: short bounce, settles pressed at 604
    800,                          // release 2: clean
};

static bool raw_signal(int t) {
    bool level = false;
    for (size_t i = 0; i < sizeof toggles / sizeof toggles[0] && toggles[i] <= t; i++) level = !level;
    return level;
}

int main(void) {
    Debouncer d = {0};
    int presses = 0, raw_edges = 0;
    bool prev_raw = false;
    for (int t = 0; t < 1000; t++) {
        bool raw = raw_signal(t);
        if (raw && !prev_raw) raw_edges++;
        prev_raw = raw;
        if (debounce(&d, raw)) {
            presses++;
            printf("t=%3d ms: PRESS #%d\n", t, presses);
        }
    }
    printf("raw rising edges: %d, debounced presses: %d → %s\n", raw_edges, presses, presses == 2 ? "correct" : "WRONG");
    return presses == 2 ? 0 : 1;
}
