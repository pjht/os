/**
 * \file 
*/

#include <cpu/ports.h>

static char configured[]={0,0,0,0}; //!< What serial ports have been detected and configured
static int data_ports[4]={0x3f8,0x2f8,0x3e8,0x2e8}; //!< List of the data ports for all the potential serial ports

#define data_port(com) (data_ports[com]) //!< Returns the data port of a serial port
#define int_port(com) (data_port(com)+1) //!< Returns the interrupt config port of a serial port
#define fifo_port(com) (data_port(com)+2) //!< Returns the fifo config port of a serial port
#define line_cmd_port(com) (data_port(com)+3) //!< Returns the line cmd port of a serial port
#define modem_cmd_port(com) (data_port(com)+4) //!< Returns the modem cmd port of a serial port
#define line_stat_port(com) (data_port(com)+5) //!< Returns the line status port of a serial port
#define scratch_port(com) (data_port(com)+7) //!< Returns the scratch port of a serial port
#define is_transmit_fifo_empty(com) (port_byte_in(line_stat_port(com))&0x20) //!< Returns whether the trasmit FIFO is empty.

/**
 * Configure a serial port with a specified baud rate.
 * \param com The number of the serial port to configure
 * \param rate The baud rate to set the serial port to. 
*/
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
