/**
 * \file 
*/

#include <cpu/ports.h>

static char configured[]={0,0,0,0}; //!< What serial ports have been detected and configured
static int data_ports[4]={0x3f8,0x2f8,0x3e8,0x2e8}; //!< List of the data ports for all the potential serial ports

#define DATA_PORT(com) (data_ports[com]) //!< Returns the data port of a serial port
#define INT_PORT(com) (DATA_PORT(com)+1) //!< Returns the interrupt config port of a serial port
#define FIFO_PORT(com) (DATA_PORT(com)+2) //!< Returns the fifo config port of a serial port
#define LINE_CMD_PORT(com) (DATA_PORT(com)+3) //!< Returns the line cmd port of a serial port
#define MODEM_CMD_PORT(com) (DATA_PORT(com)+4) //!< Returns the modem cmd port of a serial port
#define LINE_STAT_PORT(com) (DATA_PORT(com)+5) //!< Returns the line status port of a serial port
#define SCRATCH_PORT(com) (DATA_PORT(com)+7) //!< Returns the scratch port of a serial port
#define IS_TRANSMIT_FIFO_EMPTY(com) (port_byte_in(LINE_STAT_PORT(com))&0x20) //!< Returns whether the trasmit FIFO is empty.
#define IS_RECEIVE_FIFO_READY(com) (port_byte_in(LINE_STAT_PORT(com))&0x1) //!< Returns whether the receive FIFO is empty.

/**
 * Configure a serial port with a specified baud rate.
 * \param com The number of the serial port to configure
 * \param rate The baud rate to set the serial port to. 
*/
static void configure(int com, int rate) {
  configured[com]=1;
  port_byte_out(LINE_CMD_PORT(com),0x80); // Enable DLAB
  port_byte_out(DATA_PORT(com),((115200/rate)>>8)&0xFF); //Write high byte of divisor
  port_byte_out(DATA_PORT(com),(115200/rate)&0xFF); //Write low byte of divisor
  port_byte_out(LINE_CMD_PORT(com),0x03); //Disable DLAB and set 8N1 trasmission mode 
  port_byte_out(FIFO_PORT(com),0xC7); //Enable & clear FIFOs and set Data Ready interrupt level to 14.
  port_byte_out(MODEM_CMD_PORT(com),0x03); //Enable DTR and RTS
  port_byte_out(INT_PORT(com),0x0); //Disable interrupts
  serial_printf("Port %d configured\n",com);
}

void serial_init() {
  port_byte_out(SCRATCH_PORT(0),0xaa);
  if (port_byte_in(SCRATCH_PORT(0))==0xaa) {
    configure(0,9600);
  }
  port_byte_out(SCRATCH_PORT(1),0xaa);
  if (port_byte_in(SCRATCH_PORT(1))==0xaa) {
    configure(1,9600);
  }
}

void serial_putc(char c) {
  if (!configured[0]) return;
  if (c=='\n') serial_putc('\r');
  while(!IS_TRANSMIT_FIFO_EMPTY(0));
  port_byte_out(DATA_PORT(0),c);
}

char serial_getc() {
  /* serial_printf("Port 0 byte read requested\n"); */
  if (!configured[0]) return 0;
  /* serial_printf("Port 0 reading byte\n"); */
  while(!IS_RECEIVE_FIFO_READY(0));
  /* serial_printf("Port 0 read byte\n"); */
  return port_byte_in(DATA_PORT(0));
}

void serial_putc_port(char c, int port) {
  if (!configured[port]) return;
  if (c=='\n') serial_putc('\r');
  while(!IS_TRANSMIT_FIFO_EMPTY(port));
  port_byte_out(DATA_PORT(port),c);
}

char serial_getc_port(int port) {
  serial_printf("Port %d byte read requested\n",port);
  if (!configured[port]) return 0;
  serial_printf("Port %d reading byte\n",port);
  while(!IS_RECEIVE_FIFO_READY(port));
  char data = port_byte_in(DATA_PORT(port));
  serial_printf("Port %d read byte \"%c\" (%x)\n",port, data, data); 
  return data;
}
