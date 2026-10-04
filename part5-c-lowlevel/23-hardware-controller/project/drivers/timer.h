#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>

void timer_init(uint32_t tick_us); // 1000 → one interrupt per millisecond
uint32_t millis(void);             // wraps after ~49 days: compare with (now - then) >= interval

#endif
