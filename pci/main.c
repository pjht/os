#include "pci.h"
#include <stdio.h>

int main() {
  stdout=fopen("/dev/vga","w");
  pci_init();
}
