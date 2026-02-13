/* hello.c */

extern void PUT32(unsigned int, unsigned int);
extern unsigned int GET32(unsigned int);

#define UART0_BASE     0x44E09000
#define UART_THR       (UART0_BASE + 0x00)   // Transmitter Holding Register
#define UART_LSR       (UART0_BASE + 0x14)   // Line Status Register
#define UART_LSR_THRE  0x20                  // Transmit Holding Register Empty

void uart_send(unsigned char x) {
    while ((GET32(UART_LSR) & UART_LSR_THRE) == 0);  // Wait for THR empty
    PUT32(UART_THR, x);
}

void uart_puts(const char *s) {
    while (*s) {
        uart_send(*s++);
    }
}

int main(void) {
    uart_puts("Hello World\n");
    while (1);  // Prevent exit
    return 0;
}
