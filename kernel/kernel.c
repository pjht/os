#include "../drivers/vga.h"
#include "../drivers/serial.h"
#include "../drivers/isr.h"
#include "../drivers/gdt.h"
#include "../drivers/keyboard.h"
#include "../libc/string.h"
#include "../libc/memory.h"
#include "syscalls.h"
#include "multiboot.h"

typedef struct {
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
} tar_header;

#define MMAP_ENTRIES 5
#define VIRT_OFFSET 0xC0000000
uint32_t total_mb;
uint32_t mem_map[MMAP_ENTRIES+1][2];
tar_header *headers[32];
char* tar_file;
void switch_to_user_mode() {
  asm volatile("  \
    cli; \
    mov $0x23, %ax; \
    mov %ax, %ds; \
    mov %ax, %es; \
    mov %ax, %fs; \
    mov %ax, %gs; \
                  \
    mov %esp, %eax; \
    pushl $0x23; \
    pushl %eax; \
    pushf; \
    pop %eax; \
    or $0x200,%eax; \
    push %eax; \
    pushl $0x1B; \
    push $1f; \
    iret; \
  1: \
    ");
}
void halt() {
  asm volatile("cli;\
  hltlabel: hlt;\
  jmp hltlabel");
}

void get_memory(multiboot_info_t* mbd) {
  if ((mbd->flags&MULTIBOOT_INFO_MEM_MAP)!=0) {
    uint32_t mmap_length=mbd->mmap_length;
    struct multiboot_mmap_entry* mmap_addr=(struct multiboot_mmap_entry*)(mbd->mmap_addr+VIRT_OFFSET);
    uint32_t size;
    struct multiboot_mmap_entry* mmap_entry=mmap_addr;
    int i;
    for (i=0;(uint32_t)mmap_entry<((uint32_t)mmap_addr+mmap_length);mmap_entry=(struct multiboot_mmap_entry*)((uint32_t)mmap_entry+size+4)) {
      if (i>=MMAP_ENTRIES) {
        break;
      }
      size=mmap_entry->size;
      uint32_t start_addr=mmap_entry->addr;
      uint32_t length=mmap_entry->len;
      uint32_t type=mmap_entry->type;
      if (type!=1) {
        continue;
      }
      mem_map[i][0]=start_addr;
      mem_map[i][1]=start_addr+length-1;
      i++;
    }
    mem_map[i][0]=0;
    mem_map[i][0]=0;
    total_mb=0;
  } else if ((mbd->flags&MULTIBOOT_INFO_MEMORY)!=0) {
    total_mb=((mbd->mem_upper)/1024)+2;
    mem_map[0][0]=0;
    mem_map[0][1]=0;
  } else {
    write_string("Cannot detect memory. Halting");
    halt();
  }
}

void print_memory() {
  char str[100];
  if (total_mb>0) {
    if (total_mb%1024==0) {
      int_to_ascii(total_mb/1024,str);
    } else {
      int_to_ascii(total_mb,str);
    }
    write_string(str);
    if (total_mb%1024==0) {
      write_string(" GB ");
    } else {
      write_string(" MB ");
      write_string("of memory detected\n");
    }
  } else {
    for (int i=0;i<MMAP_ENTRIES;i++) {
      if (mem_map[i][0]==0&&mem_map[i][1]==0) {
        break;
      }
      char str[100];
      write_string("Memory from ");
      str[0]='\0';
      hex_to_ascii(mem_map[i][0],str);
      write_string(str);
      write_string(" to ");
      str[0]='\0';
      hex_to_ascii(mem_map[i][1],str);
      write_string(str);
      write_string("\n");
    }
  }
}

uint32_t getsize(const char *in) {
    uint32_t size=0;
    uint32_t j;
    uint32_t count=1;
    for (j=11;j > 0;j--,count*=8) {
      size+=((in[j-1]-'0')*count);
    }
    return size;
}

void main(multiboot_info_t* mbd, uint32_t magic) {
  uint32_t tmp_mbd=(uint32_t)mbd;
  tmp_mbd+=VIRT_OFFSET;
  mbd=(multiboot_info_t*)tmp_mbd;
	init_vga(WHITE,BLACK);
  write_string("Initialized VGA\n");
  if (magic!=MULTIBOOT_BOOTLOADER_MAGIC) {
    write_string("Multiboot magic number is incorrect. Halting.");
    halt();
  }
	serial_full_configure(SERIAL_COM1_BASE,12);
	write_string("Initialized COM1 at 9600 baud\n");
	isr_install();
	asm volatile("sti");
  write_string("Setup interrupts\n");
  init_gdt();
  write_string("Setup new GDT\n");
  if ((mbd->flags&MULTIBOOT_INFO_CMDLINE)!=0) {
    write_string("Command line:");
    write_string((char*)(mbd->cmdline+VIRT_OFFSET));
    write_string("\n");
  }
  get_memory(mbd);
  print_memory();
  if ((mbd->flags&MULTIBOOT_INFO_MODS)!=0) {
    uint32_t mods_count=mbd->mods_count;
    if (mods_count>0) {
      while (mods_count>0) {
        multiboot_module_t* mods_addr=(multiboot_module_t*)(mbd->mods_addr+VIRT_OFFSET);
        if (strcmp((char*)(mods_addr->cmdline+VIRT_OFFSET),"initrd.tar")==0) {
          tar_file=mods_addr->mod_start+VIRT_OFFSET;
        };
        mods_count--;
      }
    }
  }
  for (uint32_t i=0;;i++) {
      tar_header* header=(tar_header*)tar_file;
      if (header->filename[0]=='\0') {
        break;
      }
      uint32_t size=getsize(header->size);
      headers[i]=header;
      tar_file+=((size/512)+1)*512;
      if (size%512) {
        tar_file+=512;
      }
  }
  for (int i=0;i<32;i++) {
    if (headers[i]==0) {
      break;
    }
    write_string("Got file ");
    write_string(headers[i]->filename);
    write_string("\n");
    write_string("Contents:\n");
    write_string((char*)(((uint32_t)headers[i])+512));
    write_string("\n");
  }
	// init_keyboard();
	// write_string("Keyboard initialized\n");
	// switch_to_user_mode();
	// syscall_write_string("MYOS V 1.0\n");
	// while (1) {};
}

void user_input(char* buf) {};
void kgets(char* buf) {};
