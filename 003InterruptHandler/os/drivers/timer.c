#include "timer.h"

int isTick = 0;

/**
 * Initialization of the Timer 2
 * @param time_ns: timer duration in nano seconds 
 */
void timer_init(int time_ns) {
    // Enable timer clock
    PUT32(CM_PER_TIMER2_CLKCTRL, 0x2);
    // Unmask IRQ 68
    PUT32(INTC_MIR_CLEAR2, (1 << 4));
    // Config interrupt priority
    PUT32(INTC_ILR68, 0x0);
    // Stop the timer
    PUT32(TCLR, 0x0);
    // Clear pending interrupts
    PUT32(TISR, 0x7);
    // Calculate the timer load value
    int load_value = ~0 - (time_ns / 1000) * TIMER_FREQ;
    PUT32(TLDR, load_value);
    PUT32(TCRR, load_value);
    // Enable overflow interrupt
    PUT32(TIER, 0x2);
    // Start in auto-reload mode
    PUT32(TCLR, 0x3);
}

/**
 * Handler of the Timer 2 interrupt
 */
void timer_irq_handler(void) {
    // Clear timer interrupt flag
    PUT32(TISR, 0x2);
    PUT32(INTC_CONTROL, 0x1);
    isTick = 1;
}