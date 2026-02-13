#ifndef TIMER_H_
#define TIMER_H_

#include "os.h"
#include "uart.h"

extern int isTick;

void timer_init(int time_ns);
void timer_irq_handler(void);

#endif  /* TIMER_H_ */