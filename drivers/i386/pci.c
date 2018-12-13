#include <stdint.h>
#include "../../cpu/i386/ports.h"
#include "../../kernel/klog.h"
#include "../pci.h"
#include "pci.h"
#include "../../libc/stdlib.h"

pci_dev_common_info** pci_devs;
uint32_t pci_max_devs;
uint32_t pci_num_devs;

uint32_t pci_read_config(uint8_t bus,uint8_t device,uint8_t func,uint8_t offset) {
  uint32_t address;
  uint32_t lbus=(uint32_t)bus;
  uint32_t ldev=(uint32_t)device;
  uint32_t lfunc=(uint32_t)func;
  uint16_t tmp=0;
  address=(uint32_t)((lbus << 16)|(ldev << 11)|(lfunc<<8)|(offset&0xfc)|((uint32_t)0x80000000));
  port_long_out(PCI_CONFIG_ADDRESS,address);
  uint32_t data=port_long_in(PCI_CONFIG_DATA);
  return data;
}

pci_dev_common_info* pci_get_dev_info(uint8_t bus,uint8_t device,uint8_t func) {
  uint32_t* info=malloc(sizeof(uint32_t)*4);
  info[0]=pci_read_config(bus,device,func,0);
  info[1]=pci_read_config(bus,device,func,4);
  info[2]=pci_read_config(bus,device,func,8);
  info[3]=pci_read_config(bus,device,func,0xC);
  return (pci_dev_common_info*)info;
}

void pci_checkFunction(pci_dev_common_info* info);

void pci_checkDevice(uint8_t bus, uint8_t device) {
  pci_dev_common_info* info=pci_get_dev_info(bus,device,0);
  if(info->vend_id==0xFFFF||info->class_code==0xFF) {
    return;
  }
  pci_checkFunction(info);
  if((info->header_type&0x80)!=0) {
    for(uint8_t function=1;function<8;function++) {
      pci_dev_common_info* info=pci_get_dev_info(bus,device,function);
      if(info->vend_id!=0xFFFF&&info->class_code!=0xFF) {
        pci_checkFunction(info);
      }
    }
  }
}

void pci_checkFunction(pci_dev_common_info* info) {
  if (pci_num_devs==pci_max_devs) {
    pci_max_devs+=32;
    pci_devs=malloc(sizeof(pci_dev_common_info)*pci_max_devs);
  }
  klog("INFO","Found PCI device. Class code:%x, Subclass:%x Prog IF:%x",info->class_code,info->subclass,info->prog_if);
  pci_devs[pci_num_devs]=info;
  pci_num_devs++;
}

void pci_init() {
  pci_devs=malloc(sizeof(pci_dev_common_info)*32);
  pci_max_devs=32;
  pci_num_devs=0;
  for(uint16_t bus=0;bus<1; bus++) {
    for(uint8_t device=0;device<32; device++) {
        pci_checkDevice(bus, device);
    }
  }
}
