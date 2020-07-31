/**
 * \file
*/

#include "tasking.h"
#include "cpu/isr.h"

/**
 * Interrupt handler for the timer interrupt
 * \param r The saved state of the CPU when the interrupt occured
*/
void timer_handler(registers_t* r) {
  tasking_yield();
}
