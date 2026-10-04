#include "uart.h"

#include <stdarg.h>
#include <stdio.h>

#include "fc1.h"
#include "ringbuf.h"

static RingBuf rx_queue; // filled by UART_IRQHandler, drained by uart_getc
static volatile uint32_t overruns;

void uart_init(void) {
    UART->ICR = UART_SR_RXNE | UART_SR_ORE;
    UART->CR = UART_CR_EN | UART_CR_RXIE;
}

void uart_putc(char c) {
    while (UART->CMD & UART_CMD_SEND) {
        // busy-wait: the previous byte hasn't been taken by the hardware yet
    }
    UART->TXDR = (uint8_t)c;
    UART->CMD = UART_CMD_SEND;
}

void uart_puts(const char* s) {
    while (*s) {
        if (*s == '\n') uart_putc('\r'); // serial terminals expect CR LF
        uart_putc(*s++);
    }
}

void uart_printf(const char* fmt, ...) {
    char buf[160];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof buf, fmt, args); // truncates safely if the message is too long
    va_end(args);
    uart_puts(buf);
}

bool uart_getc(char* c) {
    uint8_t byte;
    if (!ringbuf_pop(&rx_queue, &byte)) return false;
    *c = (char)byte;
    return true;
}

uint32_t uart_rx_dropped(void) { return rx_queue.dropped + overruns; }

// Interrupt handler: keep it SHORT. Grab the byte, queue it, acknowledge, return.
// No printf, no blocking, no heap in here.
void UART_IRQHandler(void) {
    uint32_t sr = UART->SR;
    if (sr & UART_SR_RXNE) {
        ringbuf_push(&rx_queue, (uint8_t)UART->RXDR);
        UART->ICR = UART_SR_RXNE;
    }
    if (sr & UART_SR_ORE) {
        overruns++;
        UART->ICR = UART_SR_ORE;
    }
}
