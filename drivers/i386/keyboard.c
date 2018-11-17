#include "../keyboard.h"
#include "../../cpu/i386/ports.h"
#include "../screen.h"
#include "../../cpu/i386/isr.h"
#include "../../libc/string.h"
#include "../../kernel/kernel.h"
#include <stdint.h>

#define BACKSPACE 0x0E
#define ENTER 0x1C
#define CAPS 0x3A
#define LSHIFT 0x2A
#define RSHIFT 0x36
#define LSHIFTUP 0xAA
#define RSHIFTUP 0xB6
static char key_buffer[256];

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

static void keyboard_callback(registers_t regs) {
    uint8_t scancode=port_byte_in(0x60);
    if (scancode==BACKSPACE) {
      backspace(key_buffer);
      screen_backspace();
    } else if (scancode==CAPS) {
      caps=!caps;
    } else if (scancode==LSHIFT || scancode==RSHIFT) {
      shift=1;
    } else if (scancode==LSHIFTUP || scancode==RSHIFTUP) {
      shift=0;
    } else if (scancode==ENTER) {
      write_string("\n");
      append(key_buffer,'\n');
      user_input(key_buffer);
      key_buffer[0]='\0';
    } else if (scancode <=58) {
      char letter;
      if (shift) {
        letter=sc_shift_ascii[(int)scancode];
      } else if (caps) {
        letter=sc_caps_ascii[(int)scancode];
      } else {
        letter=sc_ascii[(int)scancode];
      }
      char str[2]={letter,'\0'};
      append(key_buffer,letter);
      write_string(str);
    }
}

void init_keyboard() {
  register_interrupt_handler(IRQ1, keyboard_callback);
}
