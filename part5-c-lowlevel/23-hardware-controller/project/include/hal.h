// hal.h: the few things that differ between real silicon and the host simulator.
#ifndef HAL_H
#define HAL_H

#include <stdbool.h>

// Prepare the platform. On the host this starts the simulator thread and parses
// options; on a microcontroller it would configure clocks and memory.
void hal_init(int argc, char** argv);

// Firmware main loops never end on real hardware. The simulator ends the run when
// its script is finished, so the host build can exit cleanly.
bool hal_running(void);

#ifdef FC1_SIMULATOR
// Host: implemented by sim/fc1_sim.c with a mutex that the "interrupt" thread also takes.
void irq_disable(void);
void irq_enable(void);
void wait_for_interrupt(void);
#else
// Cortex-M: single instructions (see chapter 24 for how inline asm works).
static inline void irq_disable(void) { __asm__ volatile("cpsid i" ::: "memory"); }
static inline void irq_enable(void) { __asm__ volatile("cpsie i" ::: "memory"); }
static inline void wait_for_interrupt(void) { __asm__ volatile("wfi"); } // sleep until an IRQ
#endif

#endif
