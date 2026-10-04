// Bit manipulation: the everyday toolkit of driver code.
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define BIT(n) (1u << (n))
// A field of `width` bits starting at bit `pos`.
#define FIELD_MASK(pos, width) ((((uint32_t)1 << (width)) - 1u) << (pos))
#define FIELD_GET(reg, pos, width) (((reg) & FIELD_MASK(pos, width)) >> (pos))
#define FIELD_SET(reg, pos, width, val) (((reg) & ~FIELD_MASK(pos, width)) | (((uint32_t)(val) << (pos)) & FIELD_MASK(pos, width)))

static void print_bin(const char* label, uint32_t v) {
    printf("%-26s 0b", label);
    for (int i = 15; i >= 0; i--) {
        putchar((v >> i) & 1 ? '1' : '0');
        if (i % 4 == 0 && i) putchar('_');
    }
    printf("  (0x%04" PRIX32 ")\n", v);
}

int main(void) {
    uint32_t reg = 0;
    print_bin("start", reg);
    reg |= BIT(3);
    print_bin("set bit 3      |= BIT(3)", reg);
    reg |= BIT(0) | BIT(7);
    print_bin("set bits 0, 7", reg);
    reg &= ~BIT(0);
    print_bin("clear bit 0    &= ~BIT(0)", reg);
    reg ^= BIT(15);
    print_bin("toggle bit 15  ^= BIT(15)", reg);
    printf("%-26s %s\n", "test bit 7     & BIT(7)", (reg & BIT(7)) ? "set" : "clear");

    // Multi-bit fields. Example: a 2-bit "mode" field at bits 10-11, as in STM32 GPIO MODER.
    reg = FIELD_SET(reg, 10, 2, 2);
    print_bin("mode field (10..11) = 2", reg);
    printf("%-26s %" PRIu32 "\n", "read mode field back", FIELD_GET(reg, 10, 2));
    reg = FIELD_SET(reg, 10, 2, 1); // read-modify-write: only those 2 bits change
    print_bin("mode field (10..11) = 1", reg);

    // Useful idioms
    uint32_t x = 0x00F0;
    printf("\nlowest set bit of 0x%04" PRIX32 ": x & -x = 0x%04" PRIX32 "\n", x, x & -x);
    printf("is 64 a power of two? %s\n", (64 & (64 - 1)) == 0 ? "yes" : "no");
    printf("popcount(0xF0F0) = %d (number of set bits)\n", __builtin_popcount(0xF0F0));
    printf("count trailing zeros of 0x0100 = %d (index of the lowest set bit)\n", __builtin_ctz(0x0100));

    // Endianness: how a 32-bit value is laid out in memory byte by byte.
    uint32_t word = 0x11223344;
    const uint8_t* bytes = (const uint8_t*)&word;
    printf("0x11223344 in memory: %02X %02X %02X %02X → %s-endian\n", bytes[0], bytes[1], bytes[2], bytes[3],
           bytes[0] == 0x44 ? "little" : "big");
    uint32_t swapped = __builtin_bswap32(word); // network protocols are big-endian
    printf("byte-swapped: 0x%08" PRIX32 "\n", swapped);

    // Signed shifts are a trap: right-shifting a negative value is implementation-defined,
    // and left-shifting into the sign bit is undefined. Use unsigned types for registers.
    int8_t s = -16;
    printf("(-16 >> 2) = %d on this compiler (arithmetic shift), but don't rely on it\n", s >> 2);
    return 0;
}
