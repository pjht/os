#include <stdint.h>
#include "../../cpu/i386/ports.h"
#include <klog.h>
#include "pci.h"
#include "../vga.h"
#include <stdlib.h>

pci_dev_common_info** pci_devs;
static uint32_t max_devs;
uint32_t pci_num_devs;

static uint32_t read_config(uint8_t bus,uint8_t device,uint8_t func,uint8_t offset) {
  uint32_t address;
  uint32_t lbus=(uint32_t)bus;
  uint32_t ldev=(uint32_t)device;
  uint32_t lfunc=(uint32_t)func;
  address=(uint32_t)((lbus << 16)|(ldev << 11)|(lfunc<<8)|(offset&0xfc)|((uint32_t)0x80000000));
  port_long_out(PCI_CONFIG_ADDRESS,address);
  uint32_t data=port_long_in(PCI_CONFIG_DATA);
  return data;
}

static void write_config(uint8_t bus,uint8_t device,uint8_t func,uint8_t offset,uint32_t data) {
  uint32_t address;
  uint32_t lbus=(uint32_t)bus;
  uint32_t ldev=(uint32_t)device;
  uint32_t lfunc=(uint32_t)func;
  address=(uint32_t)((lbus << 16)|(ldev << 11)|(lfunc<<8)|(offset&0xfc)|((uint32_t)0x80000000));
  port_long_out(PCI_CONFIG_ADDRESS,address);
  port_long_out(PCI_CONFIG_DATA,data);
}

pci_dev_common_info* pci_get_dev_info(uint8_t bus,uint8_t device,uint8_t func) {
  uint32_t* info=malloc(sizeof(uint32_t)*5);
  info[0]=read_config(bus,device,func,0);
  info[1]=read_config(bus,device,func,4);
  info[2]=read_config(bus,device,func,8);
  info[3]=read_config(bus,device,func,0xC);
  pci_dev_common_info* pci_info=(pci_dev_common_info*)info;
  pci_info->bus=bus;
  pci_info->device=device;
  pci_info->func=func;
  return pci_info;
}

void pci_set_dev_info(pci_dev_common_info* inf) {
  uint32_t* info=(uint32_t*)inf;
  write_config(inf->bus,inf->device,inf->func,0,info[0]);
  write_config(inf->bus,inf->device,inf->func,4,info[1]);
  write_config(inf->bus,inf->device,inf->func,8,info[2]);
  write_config(inf->bus,inf->device,inf->func,0xC,info[3]);
}

static void checkFunction(pci_dev_common_info* info);

static void checkDevice(uint8_t bus, uint8_t device) {
  pci_dev_common_info* info=pci_get_dev_info(bus,device,0);
  if(info->vend_id==0xFFFF||info->class_code==0xFF) {
    return;
  }
  checkFunction(info);
  if((info->header_type&0x80)!=0) {
    for(uint8_t function=1;function<8;function++) {
      pci_dev_common_info* info=pci_get_dev_info(bus,device,function);
      if(info->vend_id!=0xFFFF&&info->class_code!=0xFF) {
        checkFunction(info);
      }
    }
  }
}

static void checkFunction(pci_dev_common_info* info) {
  if (pci_num_devs==max_devs) {
    max_devs+=32;
    pci_devs=malloc(sizeof(pci_dev_common_info)*max_devs);
  }
  // pci_devs[pci_num_devs]=info;
  // pci_num_devs++;
  klog("INFO","Found PCI device. Class code:%x, Subclass:%x Prog IF:%x",info->class_code,info->subclass,info->prog_if);
  klog("INFO","Vendor ID:%x, Device ID:%x",info->vend_id,info->dev_id);
  if ((info->header_type&0x7f)==0) {
    for (uint8_t offset=0x10;offset<0x10+4*6;offset+=4) {
      uint32_t bar=read_config(info->bus,info->device,info->func,offset);
      if (bar!=0) {
        if (bar&0x1) {
          klog("INFO","IO BAR%d:%x",(offset-0x10)/4,bar&0xFFFFFFFC);
        } else {
          klog("INFO","MEM BAR%d:%x",(offset-0x10)/4,bar&0xFFFFFFF0);
        }
      }
    }
  }
}

void pci_init() {
  pci_devs=malloc(sizeof(pci_dev_common_info)*32);
  max_devs=32;
  pci_num_devs=0;
  for(uint16_t bus=0;bus<1; bus++) {
    for(uint8_t device=0;device<32; device++) {
        checkDevice(bus, device);
    }
  }
}
