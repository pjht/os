#include "pci.h"
#include <stdio.h>

int main() {
  FILE* file;
  do {
    file=fopen("/dev/vga","w");
  } while(file==NULL);
  do {
    file=fopen("/dev/vga","w");
  } while(file==NULL);
  yield();
  pci_init();
}
