#ifndef OS_H_
#define OS_H_

#include "float.h"

#define UART0_BASE 0x101f1000

#define UART_DR      0x00  // Data Register
#define UART_FR      0x18  // Flag Register
#define UART_FR_TXFF 0x20  // Transmit FIFO Full
#define UART_FR_RXFE 0x10  // Receive FIFO Empty

void uart_putc(char c);
char uart_getc();
void uart_puts(const char *s);
void uart_gets_input(char *buffer, int max_length);
int uart_get_until_whitespace(char *buffer, int max_length, int *endsInNL);
int uart_get_match(const char match);

#endif  /* OS_H_ */