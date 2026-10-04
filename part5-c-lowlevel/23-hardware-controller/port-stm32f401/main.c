// Bare-metal firmware for the Nucleo-F401RE: no HAL library, no libc, just registers.
//
//  - LD2 (green LED, PA5) blinks: 1 Hz normally, 5 Hz while the blue button (B1, PC13) is held
//  - USART2 (PA2/PA3, wired to the ST-LINK's virtual COM port) prints the uptime every second
//    and echoes what you type, at 115200 baud
//  - SysTick interrupts every 1 ms. Received bytes are queued by the USART2 interrupt.
//
// Build & flash: see the Makefile. All addresses come from the reference manual RM0368
// and the Cortex-M4 generic user guide.
#include <stdbool.h>
#include <stdint.h>

#include "../project/include/ringbuf.h" // the same lock-free queue as the simulator project

#define REG32(addr) (*(volatile uint32_t*)(addr))

// RCC: reset & clock control (RM0368 §6.3)
#define RCC_BASE 0x40023800u
#define RCC_AHB1ENR REG32(RCC_BASE + 0x30)
#define RCC_APB1ENR REG32(RCC_BASE + 0x40)
#define RCC_AHB1ENR_GPIOAEN (1u << 0)
#define RCC_AHB1ENR_GPIOCEN (1u << 2)
#define RCC_APB1ENR_USART2EN (1u << 17)

// GPIO (RM0368 §8.4)
typedef struct {
    volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFR[2];
} GPIO_TypeDef;
#define GPIOA ((GPIO_TypeDef*)0x40020000u)
#define GPIOC ((GPIO_TypeDef*)0x40020800u)
#define LED_PIN 5u     // PA5
#define BUTTON_PIN 13u // PC13, active low (external pull-up on the board)

// USART2 (RM0368 §19.6)
typedef struct {
    volatile uint32_t SR, DR, BRR, CR1, CR2, CR3, GTPR;
} USART_TypeDef;
#define USART2 ((USART_TypeDef*)0x40004400u)
#define USART_SR_RXNE (1u << 5)
#define USART_SR_TXE (1u << 7)
#define USART_CR1_RE (1u << 2)
#define USART_CR1_TE (1u << 3)
#define USART_CR1_RXNEIE (1u << 5)
#define USART_CR1_UE (1u << 13)

// Cortex-M core peripherals: SysTick and the NVIC (interrupt controller)
#define SYST_CSR REG32(0xE000E010u)
#define SYST_RVR REG32(0xE000E014u)
#define SYST_CVR REG32(0xE000E018u)
#define NVIC_ISER(n) REG32(0xE000E100u + 4u * (n))
#define USART2_IRQn 38u

#define CPU_HZ 16000000u // after reset the chip runs on its internal 16 MHz HSI oscillator

static volatile uint32_t ms_ticks;
static RingBuf rx;

void SysTick_Handler(void) { ms_ticks++; }

void USART2_IRQHandler(void) {
    if (USART2->SR & USART_SR_RXNE) ringbuf_push(&rx, (uint8_t)USART2->DR); // reading DR clears RXNE
}

static void uart_putc(char c) {
    while (!(USART2->SR & USART_SR_TXE)) {
    }
    USART2->DR = (uint8_t)c;
}

static void uart_puts(const char* s) {
    while (*s) uart_putc(*s++);
}

static void uart_put_uint(uint32_t v) { // no printf without libc: format digits by hand
    char buf[11];
    int i = 0;
    do {
        buf[i++] = (char)('0' + v % 10);
        v /= 10;
    } while (v);
    while (i) uart_putc(buf[--i]);
}

static void clocks_init(void) {
    RCC_AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN; // peripherals are unpowered until enabled
    RCC_APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC_APB1ENR; // dummy read: wait for the enable to take effect (errata ES0182)
}

static void gpio_init(void) {
    GPIOA->MODER = (GPIOA->MODER & ~(3u << (LED_PIN * 2))) | (1u << (LED_PIN * 2)); // PA5: output
    // PA2 (TX) and PA3 (RX): alternate function 7 = USART2
    GPIOA->MODER = (GPIOA->MODER & ~((3u << 4) | (3u << 6))) | (2u << 4) | (2u << 6);
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~((0xFu << 8) | (0xFu << 12))) | (7u << 8) | (7u << 12);
    GPIOC->MODER &= ~(3u << (BUTTON_PIN * 2)); // PC13: input
}

static void uart_init(void) {
    USART2->BRR = (CPU_HZ + 115200u / 2) / 115200u; // 139 = 0x8B: mantissa 8, fraction 11/16
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
    NVIC_ISER(USART2_IRQn / 32) = 1u << (USART2_IRQn % 32); // let the interrupt reach the CPU
}

static void systick_init(void) {
    SYST_RVR = CPU_HZ / 1000u - 1u; // reload value: count 16000 cycles = 1 ms
    SYST_CVR = 0;
    SYST_CSR = 0x7; // CLKSOURCE = CPU clock | TICKINT | ENABLE
}

int main(void) {
    clocks_init();
    gpio_init();
    uart_init();
    systick_init();
    uart_puts("\r\nNucleo-F401RE bare-metal demo. Type something!\r\n");

    uint32_t last_blink = 0, last_report = 0;
    bool led = false;
    for (;;) {
        uint32_t now = ms_ticks;
        bool pressed = !(GPIOC->IDR & (1u << BUTTON_PIN));
        uint32_t half_period = pressed ? 100u : 500u;
        if (now - last_blink >= half_period) {
            last_blink = now;
            led = !led;
            GPIOA->BSRR = led ? (1u << LED_PIN) : (1u << (LED_PIN + 16));
        }
        if (now - last_report >= 1000u) {
            last_report = now;
            uart_puts("uptime ");
            uart_put_uint(now / 1000u);
            uart_puts(" s\r\n");
        }
        uint8_t c;
        while (ringbuf_pop(&rx, &c)) {
            uart_puts("echo: ");
            uart_putc((char)c);
            uart_puts("\r\n");
        }
        __asm__ volatile("wfi"); // sleep until the next interrupt (SysTick wakes us every ms)
    }
}
