#include "../../cpu/i386/isr.h"
#include "../../cpu/i386/ports.h"
#define PIT_CMD_PORT 0x43
#define PIT_CHAN1_DATA 0x40
#define PIT_CHAN1_CFG_BYTE 0x36

uint64_t num_ticks=0;
int is_waiting=0;
void timer_pit_interupt(registers_t registers) {
  if (is_waiting) {
    num_ticks--;
    if (num_ticks==0) {
      is_waiting=0;
    }
  }
}

void wait(int milli) {
  num_ticks=((double)milli/1000)*20;
  is_waiting=1;
  while (is_waiting);
}

void timer_init() {
  port_byte_out(PIT_CMD_PORT,PIT_CHAN1_CFG_BYTE);
  port_byte_out(PIT_CHAN1_DATA, 0x0b);
  port_byte_out(PIT_CHAN1_DATA, 0xe9);
  register_interrupt_handler(IRQ0,timer_pit_interupt);
}
