#ifndef PCI_INTERN_H
#define PCI_INTERN_H

#include <stdint.h>
#include "../pci.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

pci_dev_common_info* pci_get_dev_info(uint8_t bus,uint8_t device,uint8_t func);
void pci_set_dev_info(pci_dev_common_info* inf);

#endif
