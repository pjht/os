#ifndef PCI_INTERN_H
#define PCI_INTERN_H

#include <stdint.h>
#include "../pci.h"
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

uint32_t pci_read_config(uint8_t bus,uint8_t device,uint8_t func,uint8_t offset);
void pci_set_dev_info(uint8_t bus,uint8_t device,uint8_t func,pci_dev_common_info* inf);

#endif
