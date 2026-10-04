#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static inline void reg_set_bits(volatile uint32_t* reg, uint32_t mask) { *reg |= mask; }
static inline void reg_clear_bits(volatile uint32_t* reg, uint32_t mask) { *reg &= ~mask; }

static inline uint32_t field_mask(unsigned width) { return width >= 32 ? 0xFFFFFFFFu : (1u << width) - 1u; }

static inline bool reg_write_field(volatile uint32_t* reg, unsigned pos, unsigned width, uint32_t value) {
    if (width == 0 || pos + width > 32 || value > field_mask(width)) return false;
    uint32_t v = *reg; // exactly one read...
    v &= ~(field_mask(width) << pos);
    v |= value << pos;
    *reg = v;          // ...and exactly one write
    return true;
}

static inline uint32_t reg_read_field(const volatile uint32_t* reg, unsigned pos, unsigned width) {
    return (*reg >> pos) & field_mask(width);
}

/* A fake W1C status register: hardware sets flags; every 1 written to it clears that flag. */
static uint32_t SR;
static void hw_apply_icr(uint32_t written) { SR &= ~written; } // what the silicon does on a write
#define FLAG_A (1u << 0)
#define FLAG_B (1u << 1)

int main(void) {
    volatile uint32_t moder = 0xA8000000; // STM32 GPIOA reset value (debug pins PA13-15 in AF mode)
    reg_write_field(&moder, 5 * 2, 2, 1); // PA5 → output
    reg_write_field(&moder, 2 * 2, 2, 2); // PA2 → alternate function
    printf("MODER = 0x%08X  PA5 mode=%u PA2 mode=%u PA13 mode=%u (unchanged)\n", (unsigned)moder,
           (unsigned)reg_read_field(&moder, 10, 2), (unsigned)reg_read_field(&moder, 4, 2),
           (unsigned)reg_read_field(&moder, 26, 2));

    volatile uint32_t afrl = 0;
    reg_write_field(&afrl, 2 * 4, 4, 7); // PA2 AF7 = USART2_TX
    reg_write_field(&afrl, 3 * 4, 4, 7); // PA3 AF7 = USART2_RX
    printf("AFRL  = 0x%08X\n", (unsigned)afrl);
    printf("write 5 into a 2-bit field → %s\n", reg_write_field(&moder, 0, 2, 5) ? "accepted?!" : "rejected");

    reg_set_bits(&afrl, 1u << 31);
    reg_clear_bits(&afrl, 1u << 31);
    printf("set+clear bit 31 → AFRL = 0x%08X\n", (unsigned)afrl);

    // The W1C trap, on a chip whose status register itself is "write 1 to clear".
    // Both flags are pending, and we want to acknowledge only A.
    SR = FLAG_A | FLAG_B;
    hw_apply_icr(FLAG_A); // correct: write ONLY the bit you mean (`SR = FLAG_A;`)
    printf("\ncorrect: SR = FLAG_A   → SR = 0x%X (B still pending)\n", (unsigned)SR);

    SR = FLAG_A | FLAG_B;
    hw_apply_icr(SR | FLAG_A); // buggy: `SR |= FLAG_A` reads 0b11 and writes 0b11 back...
    printf("buggy:   SR |= FLAG_A → SR = 0x%X (B was cleared too: its event is silently lost)\n", (unsigned)SR);
    return 0;
}
