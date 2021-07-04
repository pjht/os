#include "ps2.h"
#include <rpc.h>
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <cpu/irq.h>
#include <string.h>

#define BACKSPACE 0x0E
#define ENTER 0x1C
#define CAPS 0x3A
#define LSHIFT 0x2A
#define RSHIFT 0x36
#define LSHIFTUP 0xAA
#define RSHIFTUP 0xB6

const char *sc_name[] = { "ERROR", "Esc", "1", "2", "3", "4", "5", "6",
    "7", "8", "9", "0", "-", "=", "Backspace", "Tab", "Q", "W", "E",
        "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Lctrl",
        "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`",
        "LShift", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".",
        "/", "RShift", "Keypad *", "LAlt", "Spacebar"};
const char sc_ascii[] = { '?', '?', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '?', '?', 'q', 'w', 'e', 'r', 't', 'y',
        'u', 'i', 'o', 'p', '[', ']', '?', '?', 'a', 's', 'd', 'f', 'g',
        'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' '};
const char sc_caps_ascii[] = { '?', '?', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y',
        'U', 'I', 'O', 'P', '[', ']', '?', '?', 'A', 'S', 'D', 'F', 'G',
        'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '};
const char sc_shift_ascii[] = { '?', '?', '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y',
        'U', 'I', 'O', 'P', '{', '}', '?', '?', 'A', 'S', 'D', 'F', 'G',
        'H', 'J', 'K', 'L', ':', '"', '~', '?', '|', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', '?', '?', '?', ' '};
int caps=0;
int shift=0;
int num_irqs=0;

static void kbd_handler(int irq) {
  num_irqs++;
  char buf[128];
  int_to_ascii(num_irqs,buf);
  serial_print("Number of keyboard IRQs: ");
  serial_print(buf);
  serial_print("\n");
  uint8_t scancode=ps2_read_data();
  if (scancode==BACKSPACE) {
    putc('\b',stdout);
    putc(' ',stdout);
    putc('\b',stdout);
  } else if (scancode==CAPS) {
    caps=!caps;
  } else if (scancode==LSHIFT || scancode==RSHIFT) {
    shift=1;
  } else if (scancode==LSHIFTUP || scancode==RSHIFTUP) {
    shift=0;
  } else if (scancode==ENTER) {
    putc('\n',stdout);
  } else if (scancode<=58) {
    char letter;
    if (shift) {
      letter=sc_shift_ascii[(int)scancode];
    } else if (caps) {
      letter=sc_caps_ascii[(int)scancode];
    } else {
      letter=sc_ascii[(int)scancode];
    }
    putc(letter,stdout);
  }
  pthread_exit(NULL);
}

void init_keyboard(char port) {
  ps2_send_cmd_w_data_to_device(port,0xF0,1);
  ps2_send_cmd_to_device(port,0xF4);
  register_irq_handler(1,&kbd_handler);
  printf("[INFO] Registered keyboard driver on PS/2 port %d\n",port);
}
