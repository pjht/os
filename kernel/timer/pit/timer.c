#include "../../cpu/isr.h"
#include "../../tasking.h"
#include "../../cpu/serial.h"
#include <cpu/ports.h>

void timer_handler(registers_t* r);

void timer_init(int freq) {
  int div=1193180/freq;
  if (div>65535) {
    serial_printf("Frequency of %dHz too slow, min freq is 18 Hz\n");
    div=65535;
    freq=18;
  if (div==0) {
    serial_printf("Frequency of %dHz too slow, max freq is 1193180 Hz\n");
    div=1;
    freq=1193180;
  }
  } else {
    serial_printf("Setting PIT to %dHz using divisor of %d\n",freq,div);
  }
  isr_register_handler(0,timer_handler);
  port_byte_out(0x43,(div>>8)&0xFF);
  port_byte_out(0x40,div&0xFF);
  port_byte_out(0x40,0x2E);
}
