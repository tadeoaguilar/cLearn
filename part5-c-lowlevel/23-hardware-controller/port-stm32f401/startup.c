// startup.c: what runs before main() on a Cortex-M microcontroller.
//
// On a PC, the operating system loads your program, sets up the stack and
// zero-fills .bss. On bare metal, nobody does that for you. After reset the CPU:
//   1. loads the stack pointer from address 0 of the vector table
//   2. jumps to the address in entry 1: Reset_Handler
// Everything else (initializing globals, calling main) is this file's job.
#include <stdint.h>

// Symbols defined by the linker script. Only their ADDRESSES are meaningful.
extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _estack;

int main(void);

void Reset_Handler(void) {
    // Copy initial values of globals from flash to RAM (.data)...
    const uint32_t* src = &_sidata;
    for (uint32_t* dst = &_sdata; dst < &_edata;) *dst++ = *src++;
    // ...and zero the rest (.bss). Before this loop, `static int x;` holds garbage!
    for (uint32_t* dst = &_sbss; dst < &_ebss;) *dst++ = 0;
    main();
    for (;;) {
    } // main should never return on bare metal
}

// Any interrupt without a handler lands here. Stop, so a debugger can show where we are.
void Default_Handler(void) {
    for (;;) {
    }
}

// Weak aliases: if main.c defines SysTick_Handler, the linker uses that;
// otherwise the name falls back to Default_Handler.
#define WEAK_HANDLER(name) void name(void) __attribute__((weak, alias("Default_Handler")))
WEAK_HANDLER(NMI_Handler);
WEAK_HANDLER(HardFault_Handler);
WEAK_HANDLER(MemManage_Handler);
WEAK_HANDLER(BusFault_Handler);
WEAK_HANDLER(UsageFault_Handler);
WEAK_HANDLER(SVC_Handler);
WEAK_HANDLER(DebugMon_Handler);
WEAK_HANDLER(PendSV_Handler);
WEAK_HANDLER(SysTick_Handler);
WEAK_HANDLER(USART2_IRQHandler);

typedef void (*Handler)(void);
enum { CORE_VECTORS = 16, USART2_IRQn = 38, DEVICE_VECTORS = 85 };

// The vector table: an array of addresses at the very start of flash.
// Entries 0-15 are defined by ARM (same on every Cortex-M); the rest belong to the chip vendor.
__attribute__((section(".isr_vector"), used)) const Handler vector_table[CORE_VECTORS + DEVICE_VECTORS] = {
    [0] = (Handler)(uintptr_t)&_estack, // not a function: the initial stack pointer
    [1] = Reset_Handler,
    [2] = NMI_Handler,
    [3] = HardFault_Handler,
    [4] = MemManage_Handler,
    [5] = BusFault_Handler,
    [6] = UsageFault_Handler,
    [11] = SVC_Handler,
    [12] = DebugMon_Handler,
    [14] = PendSV_Handler,
    [15] = SysTick_Handler,
    [CORE_VECTORS + USART2_IRQn] = USART2_IRQHandler,
};
