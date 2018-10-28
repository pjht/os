#include "ports.h" /* io.h is implement in the section "Moving the cursor" */
#include "serial.h"
/* The I/O ports */

/* All the I/O ports are calculated relative to the data port. This is because
 * all serial ports (COM1, COM2, COM3, COM4) have their ports in the same
 * order, but they start at different values.
 */

#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

/* The I/O port commands */

/* SERIAL_LINE_ENABLE_DLAB:
 * Tells the serial port to expect first the highest 8 bits on the data port,
 * then the lowest 8 bits will follow
 */
#define SERIAL_LINE_ENABLE_DLAB         0x80

/** serial_configure_baud_rate:
 *  Sets the speed of the data being sent. The default speed of a serial
 *  port is 115200 bits/s. The argument is a divisor of that number, hence
 *  the resulting speed becomes (115200 / divisor) bits/s.
 *
 *  @param com      The COM port to configure
 *  @param divisor  The divisor
 */
void serial_configure_baud_rate(unsigned short com, unsigned short divisor) {
    port_byte_out(SERIAL_LINE_COMMAND_PORT(com),
         SERIAL_LINE_ENABLE_DLAB);
    port_byte_out(SERIAL_DATA_PORT(com),
         (divisor >> 8) & 0x00FF);
    port_byte_out(SERIAL_DATA_PORT(com),
         divisor & 0x00FF);
}

void serial_configure_line(unsigned short com) {
    port_byte_out(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

void serial_configure_fifo(unsigned short com) {
    port_byte_out(SERIAL_FIFO_COMMAND_PORT(com), 0xC7);
}

void serial_configure_modem(unsigned short com) {
    port_byte_out(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
}

int serial_is_transmit_fifo_empty(unsigned short com) {
    return port_byte_in(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

void serial_full_configure(unsigned short com, unsigned short divisor) {
  serial_configure_baud_rate(com,divisor);
  serial_configure_line(com);
  serial_configure_fifo(com);
  serial_configure_modem(com);
}

void serial_write_string(unsigned short com, char *str) {
  while (*str!='\0') {
    while (!serial_is_transmit_fifo_empty(com)) continue;
    if (*str=='\n') {
      port_byte_out(SERIAL_DATA_PORT(com),'\r');
      port_byte_out(SERIAL_DATA_PORT(com),'\n');
    } else {
      port_byte_out(SERIAL_DATA_PORT(com),*str);
    }
    str++;
  }
}
