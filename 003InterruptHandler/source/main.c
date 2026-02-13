#include "stdio.h"
#include "random.h"
#include "timer.h"

int main(void) {
  // TODO: Print initialization message
  // TODO: Initialize the timer using timer_init()
  // TODO: Enable interrupts using enable_irq()
  // TODO: Print a message indicating interrupts are enabled
  PRINT("---------- Starting Interrupt Handler Process ----------\n");
  PRINT("Initializing timer...\n");
  timer_init(2000);
  PRINT("Enabling interrupts...\n");
  enable_irq();
  PRINT("Timer initialized and interrupts enabled.\n");
  // Main loop: continuously print random numbers
  while (1) {
    unsigned int random_num = rand() % 1000;
    PRINT("%d\n", random_num);
    
    if (isTick) {
      PRINT("Tick\n");
      isTick = 0;
    }
    // Small delay to prevent overwhelming UART
    for (volatile int i = 0; i < 100000000; i++);
  }
  
  return 0;
}