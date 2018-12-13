#ifndef PCI_INTERN_H
#define PCI_INTERN_H

#include <stdint.h>
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

uint16_t pci_read_config_word(uint8_t bus,uint8_t slot,uint8_t func,uint8_t reg);

#endif
