// Describing hardware registers in C: structs at fixed addresses.
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// The layout of an STM32F4 GPIO port, transcribed from the reference manual (RM0368 §8.4).
typedef struct {
    volatile uint32_t MODER;   // 0x00 mode: 2 bits per pin (00 in, 01 out, 10 alternate, 11 analog)
    volatile uint32_t OTYPER;  // 0x04 output type: push-pull / open-drain
    volatile uint32_t OSPEEDR; // 0x08 output speed
    volatile uint32_t PUPDR;   // 0x0C pull-up / pull-down
    volatile uint32_t IDR;     // 0x10 input data
    volatile uint32_t ODR;     // 0x14 output data
    volatile uint32_t BSRR;    // 0x18 bit set/reset
    volatile uint32_t LCKR;    // 0x1C lock
    volatile uint32_t AFR[2];  // 0x20 alternate function low / high
} GPIO_TypeDef;

// Verify the transcription. A mistake here means writing to the wrong register,
// which fails silently.
_Static_assert(offsetof(GPIO_TypeDef, IDR) == 0x10, "IDR offset");
_Static_assert(offsetof(GPIO_TypeDef, BSRR) == 0x18, "BSRR offset");
_Static_assert(offsetof(GPIO_TypeDef, AFR) == 0x20, "AFR offset");
_Static_assert(sizeof(GPIO_TypeDef) == 0x28, "GPIO block size");

// On the chip:  #define GPIOA ((GPIO_TypeDef*)0x40020000u)
// Here we point it at an ordinary variable so the program can run on a PC.
static GPIO_TypeDef fake_gpioa;
#define GPIOA (&fake_gpioa)

// Why not C bit-fields for registers, like `struct { uint32_t mode0 : 2; ... }`?
//  - Bit order within a word is implementation-defined. It differs between compilers and ABIs.
//  - The compiler may access the register with an 8- or 16-bit instruction, while
//    many peripherals require 32-bit accesses.
//  - Every field write is a hidden read-modify-write.
// Masks and shifts are explicit, portable and auditable, so vendors use them.
#define GPIO_MODER_MASK(pin) (3u << ((pin) * 2))
#define GPIO_MODER_OUTPUT(pin) (1u << ((pin) * 2))

static void gpio_make_output(GPIO_TypeDef* port, unsigned pin) {
    uint32_t v = port->MODER;     // one read
    v &= ~GPIO_MODER_MASK(pin);   // clear the 2-bit field
    v |= GPIO_MODER_OUTPUT(pin);  // set it to 01
    port->MODER = v;              // one write
}

static void gpio_write(GPIO_TypeDef* port, unsigned pin, int high) {
    port->BSRR = high ? (1u << pin) : (1u << (pin + 16)); // atomic single write, no read needed
}

int main(void) {
    printf("GPIO register block: %zu bytes; BSRR at offset 0x%02zX\n", sizeof(GPIO_TypeDef),
           offsetof(GPIO_TypeDef, BSRR));
    printf("A real driver would use address %p for GPIOA->ODR\n", (void*)(uintptr_t)(0x40020000u + 0x14u));

    gpio_make_output(GPIOA, 5); // Nucleo-F401RE user LED (LD2) is on PA5
    printf("MODER after making PA5 an output: 0x%08X (bits 11:10 = 01)\n", (unsigned)GPIOA->MODER);
    gpio_write(GPIOA, 5, 1);
    printf("BSRR written for 'PA5 high': 0x%08X\n", (unsigned)GPIOA->BSRR);
    gpio_write(GPIOA, 5, 0);
    printf("BSRR written for 'PA5 low':  0x%08X\n", (unsigned)GPIOA->BSRR);
    printf("(on silicon the hardware applies BSRR to ODR and the LED changes; see port-stm32f401/)\n");
    return 0;
}
