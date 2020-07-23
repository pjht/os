#include <cpu/ports.h>

static char configured[]={0,0,0,0};
static int data_ports[4]={0x3f8,0x2f8,0x3e8,0x2e8};

#define data_port(com) (data_ports[com])
#define int_port(com) (data_port(com)+1)
#define fifo_port(com) (data_port(com)+2)
#define line_cmd_port(com) (data_port(com)+3)
#define modem_cmd_port(com) (data_port(com)+4)
#define line_stat_port(com) (data_port(com)+5)
#define scratch_port(com) (data_port(com)+7)
#define is_transmit_fifo_empty(com) (port_byte_in(line_stat_port(com))&0x20)

static void configure(int com, int rate) {
  configured[com]=1;
  port_byte_out(line_cmd_port(com),0x80); // Enable DLAB
  port_byte_out(data_port(com),((115200/rate)>>8)&0xFF); //Write high byte of divisor
  port_byte_out(data_port(com),(115200/rate)&0xFF); //Write low byte of divisor
  port_byte_out(line_cmd_port(com),0x03); //Disable DLAB and set 8N1 trasmission mode 
  port_byte_out(fifo_port(com),0xC7); //Enable & clear FIFOs and set Data Ready interrupt level to 14.
  port_byte_out(modem_cmd_port(com),0x03); //Enable DTR and RTS
  port_byte_out(int_port(com),0x0); //Disable interrupts
}

void serial_init() {
  port_byte_out(scratch_port(0),0xaa);
  if (port_byte_in(scratch_port(0))==0xaa) {
    configure(0,9600);
  }
}

void serial_putc(char c) {
  if (!configured[0]) return;
  if (c=='\n') serial_putc('\r');
  while(!is_transmit_fifo_empty(0));
  port_byte_out(data_port(0),c);
}
