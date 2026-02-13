#ifndef UART_H_
#define UART_H_

#include "os.h"

// Low-level OS interface functions

void os_write(const char *s);
void os_read(char *buffer, int max_length);

// Low-level OS interface functions used in stdio.h

int uart_get_until_whitespace(char *buffer, int max_length, int *endsInNL);
int uart_get_match(const char match);

// UART helper functions

void uart_putc(char c);
char uart_getc(void);

// Low-level memory access functions (implemented in root.s)

void PUT32(unsigned int addr, unsigned int value);
unsigned int GET32(unsigned int addr);

#endif  /* UART_H_ */