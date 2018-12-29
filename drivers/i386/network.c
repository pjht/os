#include "pci.h"
#include "../../cpu/i386/ports.h"
#include "../../cpu/i386/paging.h"
#include "../../libc/string.h"
#include "../../libc/stdlib.h"
void network_init(uint8_t bus,uint8_t device,uint8_t func,pci_dev_common_info* info) {
  // info->command=info->command|0x4;
  // pci_set_dev_info(bus,device,func,info);
  // free(info);
  // info=pci_get_dev_info(bus,device,func);
  // if ((info->command&0x4)==0) {
  //   klog("ERROR","Could not set network card to be PCI bus master, aborting");
  //   return;
  // }
  // uint32_t io_base=0;
  // uint32_t bar0=pci_read_config(bus,device,func,0x10);
  // if ((bar0&0x1)==1) {
  //   io_base=bar0&0xFFFFFFFB;
  // }
  // if (io_base==0) {
  //   klog("ERROR","Could not get IO base of network card, aborting");
  //   return;
  // } else {
  //   // klog("INFO","Network card IO base:%x",io_base);
  // }
  // port_byte_out(io_base+0x52,0x0);
  // // klog("INFO","Network card on");
  // // klog("INFO","Resetting network card");
  // port_byte_out(io_base+0x37,0x10);
  // while ((port_byte_in(io_base+0x37)&0x10)!=0);
  // // klog("INFO","Reset network card");
  // // void* rx_buf=malloc(sizeof(char)*9,708);
  // // void* phys=virt_to_phys(rx_buf);
  // // port_long_out(io_base+0x30,phys);
  // // klog("INFO","Setup network card recive buffer");
  // port_word_out(io_base+0x3C,0x0);
  // // klog("INFO","Setup network card interrupts");
  // port_long_out(io_base+0x44,0x2);
  // // klog("INFO","Setup network card packet filter");
  // port_byte_out(io_base+0x37,0x04);
  // // klog("INFO","Network card RX and TX enabled");
  // char* msg="";
  // char* message=malloc(sizeof(char)*(strlen(msg)+1));
  // strcpy(message,msg);
  // void* phys=virt_to_phys(message);
  // port_long_out(io_base+0x20,phys);
  // port_long_out(io_base+0x10,(strlen(msg)+1));
  // klog("INFO","Sent test message");
}
