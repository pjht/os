#ifndef PCI_H
#define PCI_H

#include <stddef.h>
#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

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

typedef struct {
  pci_dev_common_info common;
  uint32_t bar0;
  uint32_t bar1;
  uint32_t bar2;
  uint32_t bar3;
  uint32_t bar4;
  uint32_t bar5;
  uint32_t cis_ptr;
  uint16_t sub_vend_id;
  uint16_t sub_id;
  uint32_t exp_rom_addr;
  uint16_t cap_ptr;
  uint16_t reserved1;
  uint32_t reserved2;
  uint8_t int_line;
  uint8_t int_pin;
  uint8_t min_grnt;
  uint8_t max_latency;
} __attribute__((packed)) pci_dev_type0;

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
extern size_t pci_num_devs;

pci_dev_common_info* pci_get_dev_info(int bus,int device,int func);
void pci_set_dev_info(pci_dev_common_info* inf);
void pci_init();

#endif
