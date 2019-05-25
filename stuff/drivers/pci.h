#ifndef PCI_H
#define PCI_H

#include <stdint.h>

typedef struct {
  uint16_t vend_id;
  uint16_t dev_id;
  uint16_t command;
  uint16_t status;
  uint8_t rev_id;
  uint8_t prog_if;
  uint8_t subclass;
  uint8_t class_code;
  uint8_t cache_line_size;
  uint8_t lat_timer;
  uint8_t header_type;
  uint8_t bist;
  uint16_t bus;
  uint8_t device;
  uint8_t func;
} __attribute__((packed)) pci_dev_common_info;

typedef enum {
  PCI_CLASS_UNCLASSIFIED=0x0,
  PCI_CLASS_STORAGE=0x1,
  PCI_CLASS_NETWORK=0x2,
  PCI_CLASS_DISPLAY=0x3,
  PCI_CLASS_MULTIMEDIA=0x4,
  PCI_CLASS_MEMORY=0x5,
  PCI_CLASS_BRIDGE=0x6,
  PCI_CLASS_SIMPCOM=0x7,
  PCI_CLASS_BASEPERIPH=0x8,
  PCI_CLASS_INPDEV=0x9,
  PCI_CLASS_DOCK=0xa,
  PCI_CLASS_CPU=0xb,
  PCI_CLASS_SERBUS=0xc,
  PCI_CLASS_WIRELESS=0xd,
  PCI_CLASS_INTELLIGENT=0xe,
  PCI_CLASS_SATELLITE=0xf,
  PCI_CLASS_ENCRYPTION=0x10,
  PCI_CLASS_SIGPROCESS=0x11,
} pci_class;

extern pci_dev_common_info** pci_devs;
extern uint32_t pci_num_devs;
void pci_init();

#endif
