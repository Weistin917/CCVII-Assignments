#include "os.h"

volatile unsigned int * const UART0 = (unsigned int *)UART0_BASE;

// Function to send a single character via UART
void uart_putc(char c) {
    // Wait until there is space in the FIFO
    while (UART0[UART_FR / 4] & UART_FR_TXFF);
    UART0[UART_DR / 4] = c;
}

// Function to receive a single character via UART
char uart_getc() {
    // Wait until data is available
    while (UART0[UART_FR / 4] & UART_FR_RXFE);
    return (char)(UART0[UART_DR / 4] & 0xFF);
}

// Function to send a string via UART
void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

// Function to receive a line of input via UART
void uart_gets_input(char *buffer, int max_length) {
    int i = 0;
    char c;
    while (i < max_length - 1) { // Leave space for null terminator
        c = uart_getc();
        if (c == '\n' || c == '\r') {
            uart_putc('\n'); // Echo newline
            break;
        }
        uart_putc(c); // Echo character
        buffer[i++] = c;
    }
    buffer[i] = '\0'; // Null terminate the string
}

/* 
 * Function to receive input until whitespace (space, tab, line break)
 * @return the length of the received input
*/
int uart_get_until_whitespace(char *buffer, int max_length, int *endsInNL) {
    int i = 0;
    char c;
    while (i < max_length - 1) { // Leave space for null terminator
        c = uart_getc();
        if (c == '\n' || c == '\r') {
            uart_putc('\n'); // Echo newline
            *endsInNL = 1;
            break;
        } else if (c == ' ' || c == '\t') {
            uart_putc(c);
            *endsInNL = 0;
            break;
        }
        uart_putc(c); // Echo character
        buffer[i++] = c;
    }
    buffer[i++] = '\0'; // Null terminate the string
    return i;
}

/**
 * Function to receive input until a match or a mismatch (ignores white spaces)
 * @return whether there was a match or not
 */
int uart_get_match(const char match) {
    char c;
    for (;;) {
        c = uart_getc();
        uart_putc(c);
        if (!(c == '\n' || c == '\r' || c == ' ' || c == '\t')) {
            if (c == match) return 1;
            else return 0;
        }
    }
}