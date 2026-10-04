#include "timer.h"

#include "fc1.h"

// Shared between the interrupt handler and the main loop: must be volatile, so
// the main loop re-reads it instead of keeping a stale copy in a register. A
// 32-bit aligned load or store is atomic on 32/64-bit CPUs, so a lone counter
// needs no lock. Anything bigger than one word would.
static volatile uint32_t ms_ticks;

void timer_init(uint32_t tick_us) {
    TIMER->CR = 0;
    TIMER->LOAD = tick_us;
    TIMER->ICR = TIMER_SR_UIF;
    TIMER->CR = TIMER_CR_EN | TIMER_CR_IE;
}

uint32_t millis(void) { return ms_ticks; }

void TIMER_IRQHandler(void) {
    TIMER->ICR = TIMER_SR_UIF; // acknowledge first, or the interrupt fires again immediately
    ms_ticks++;
}
