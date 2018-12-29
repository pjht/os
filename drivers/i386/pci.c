#include <stdint.h>
#include "../../cpu/i386/ports.h"
#include "../../kernel/klog.h"
#include "pci.h"
#include "../../libc/stdlib.h"
#include "network.h"

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

void pci_write_config(uint8_t bus,uint8_t device,uint8_t func,uint8_t offset,uint32_t data) {
  uint32_t address;
  uint32_t lbus=(uint32_t)bus;
  uint32_t ldev=(uint32_t)device;
  uint32_t lfunc=(uint32_t)func;
  uint16_t tmp=0;
  address=(uint32_t)((lbus << 16)|(ldev << 11)|(lfunc<<8)|(offset&0xfc)|((uint32_t)0x80000000));
  port_long_out(PCI_CONFIG_ADDRESS,address);
  port_long_out(PCI_CONFIG_DATA,data);
}

pci_dev_common_info* pci_get_dev_info(uint8_t bus,uint8_t device,uint8_t func) {
  uint32_t* info=malloc(sizeof(uint32_t)*4);
  info[0]=pci_read_config(bus,device,func,0);
  info[1]=pci_read_config(bus,device,func,4);
  info[2]=pci_read_config(bus,device,func,8);
  info[3]=pci_read_config(bus,device,func,0xC);
  return (pci_dev_common_info*)info;
}

void pci_set_dev_info(uint8_t bus,uint8_t device,uint8_t func,pci_dev_common_info* inf) {
  uint32_t* info=(uint32_t*)inf;
  pci_write_config(bus,device,func,0,info[0]);
  pci_write_config(bus,device,func,4,info[1]);
  pci_write_config(bus,device,func,8,info[2]);
  pci_write_config(bus,device,func,0xC,info[3]);
}

void pci_checkFunction(uint8_t bus,uint8_t device,uint8_t func,pci_dev_common_info* info);

void pci_checkDevice(uint8_t bus, uint8_t device) {
  pci_dev_common_info* info=pci_get_dev_info(bus,device,0);
  if(info->vend_id==0xFFFF||info->class_code==0xFF) {
    return;
  }
  pci_checkFunction(bus,device,0,info);
  if((info->header_type&0x80)!=0) {
    for(uint8_t function=1;function<8;function++) {
      pci_dev_common_info* info=pci_get_dev_info(bus,device,function);
      if(info->vend_id!=0xFFFF&&info->class_code!=0xFF) {
        pci_checkFunction(bus,device,function,info);
      }
    }
  }
}

void pci_checkFunction(uint8_t bus,uint8_t device,uint8_t func,pci_dev_common_info* info) {
  if (pci_num_devs==pci_max_devs) {
    pci_max_devs+=32;
    pci_devs=malloc(sizeof(pci_dev_common_info)*pci_max_devs);
  }
  klog("INFO","Found PCI device. Class code:%x, Subclass:%x Prog IF:%x",info->class_code,info->subclass,info->prog_if);
  pci_devs[pci_num_devs]=info;
  pci_num_devs++;
  if (info->class_code==0x2&&info->subclass==0x0) {
    klog("INFO","Found network controller, detecting type");
    if (info->vend_id==0x10ec&&info->dev_id==0x8139) {
      klog("INFO","Found RTL8139, starting driver");
      network_init(bus,device,func,info);
    } else {
      klog("INFO","Unknown network card");
    }
  }
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
