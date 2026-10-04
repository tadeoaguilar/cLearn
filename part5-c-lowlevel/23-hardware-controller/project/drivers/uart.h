#ifndef UART_H
#define UART_H
#include <stdbool.h>
#include <stdint.h>

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char* s);
// printf to the serial port. Formats into a fixed stack buffer: no heap on a microcontroller.
void uart_printf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
// Non-blocking: returns false if no byte has arrived. Bytes are queued by the RX interrupt.
bool uart_getc(char* c);
uint32_t uart_rx_dropped(void);

#endif
