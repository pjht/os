#include <stdint.h>
#define NUM_ENTRIES 4

typedef struct {
  uint16_t limit_low16;
  uint16_t base_low16;
  uint8_t base_mid8;
  uint8_t access;
  uint8_t limit_flags;
  uint8_t base_high8;
} gdt_entry;

typedef struct {
  uint16_t size;
  uint32_t address;
} gdt_description;

gdt_entry gdt[NUM_ENTRIES-1];
gdt_description gdt_desc;


void gdt_init() {
  set_entry(0,0,0,0);
  set_entry(1,0,0xFFFFF,0x9E);
  set_entry(2,0,0xFFFFF,0x92);
  gdt_desc.size=(sizeof(gdt_entry)*NUM_ENTRIES)-1;
  gdt_desc.address=&gdt[0];
  asm volatile("lgdt gdt_desc");
}

void set_entry(int i,uint32_t base,uint32_t limit,uint8_t access) {
  gdt[i].limit_low16=limit&0xFFFF;
  gdt[i].base_low16=base&0xFFFFF;
  gdt[i].base_mid8=(base&0xFF0000)>>16;
  gdt[i].access=access;
  uint8_t limit_high4=(limit&0xF0000)>>16;
  gdt[i].limit_flags=0xC0&limit_high4;
  gdt[i].base_high8=(base&0xFF000000)>>24;
}
