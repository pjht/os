#ifndef ADDRESS_SPACES_H
#define ADDRESS_SPACES_H

void address_spaces_copy_data(void* cr3, void* data,uint32_t size,void* virt_addr);
void* address_spaces_put_data(void* cr3, void* data,uint32_t size);
#endif
