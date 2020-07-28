/**
 * \file 
*/

#include "cpu/arch_consts.h"
#include "cpu/halt.h"
#include "vga_err.h"
#include <grub/multiboot2.h>
#include <stdint.h>
#include <stdlib.h>

#define BMAP_LEN (NUM_FRAMES/8) //!< The size of the physical memory manager's bitmap

static char bmap[BMAP_LEN]; //!< Bitmap of allocated/non-present page frames

/**
 * Get a bit in the bitmap
 * \param index The bit to get
 * \return the bit
*/
static char get_bmap_bit(int index) {
  int byte=index/8;
  int bit=index%8;
  char entry=bmap[byte];
  return (entry&(1<<bit))>0;
}

/**
 * Set a bit in the heap bitmap
 * \param index The bit to set
*/
static void set_bmap_bit(int index) {
  int byte=index/8;
  int bit=index%8;
  bmap[byte]=bmap[byte]|(1<<bit);
}

/**
 * Clear a bit in the heap bitmap
 * \param index The bit to clear
*/
static void clear_bmap_bit(int index) {
  int byte=index/8;
  int bit=index%8;
  bmap[byte]=bmap[byte]&(~(1<<bit));
}

void pmem_init(struct multiboot_boot_header_tag* tags) {
  for (int i=0;i<BMAP_LEN;i++) {
    bmap[i]=0xFF;
  }
  char found_mmap=0;
  struct multiboot_tag* tag=(struct multiboot_tag*)(tags+1);
  while (tag->type!=0) {
    switch (tag->type) {
      case MULTIBOOT_TAG_TYPE_MMAP: {
        found_mmap=1;
        struct multiboot_mmap_entry* orig_ptr=(struct multiboot_mmap_entry*)(((char*)tag)+16);
        for (struct multiboot_mmap_entry* ptr=orig_ptr;(char*)ptr<((char*)orig_ptr)+tag->size;ptr++) {
          if (ptr->type!=MULTIBOOT_MEMORY_AVAILABLE) continue;
          size_t start=ptr->addr;
          if (start<0x100000) continue;
          size_t end=start+ptr->len-1;
          if (start&(FRAME_SZ-1)) {
            start+=FRAME_SZ;
          }
          start=start>>FRAME_NO_OFFSET;
          end=end>>FRAME_NO_OFFSET;
          for (size_t i=start;i<end;i++) {
            clear_bmap_bit(i);
          }
        }
        break;
      }
    }
    tag=(struct multiboot_tag*)((char*)tag+((tag->size+7)&0xFFFFFFF8));
  }
  if (!found_mmap) {
    vga_write_string("[PANIC] No memory map supplied by bootloader!");
    halt();
  }
  for (size_t i=0;i<NUM_KERN_FRAMES;i++) {
    set_bmap_bit(i);
  }
}

void* pmem_alloc(int num_pages) {
  size_t bmap_index;
  size_t remaining_blks;
  for(size_t i=0;i<131072;i++) {
    char got_0=0;
    remaining_blks=num_pages;
    size_t old_j;
    for (size_t j=i*8;;j++) {
      char bit=get_bmap_bit(j);
      if (got_0) {
        if (bit) {
          if (remaining_blks==0) {
              bmap_index=old_j;
              break;
          } else {
            i+=j/8;
            i--;
            break;
          }
        } else {
          remaining_blks--;
        }
      } else {
        if (!bit) {
          got_0=1;
          old_j=j;
          remaining_blks--;
        }
      }
      if (remaining_blks==0) {
        bmap_index=old_j;
        break;
      }
    }
    if (remaining_blks==0) {
      break;
    }
  }
  if (remaining_blks!=0) {
    return NULL;
  }
  for (int i=0;i<num_pages;i++) {
    set_bmap_bit(bmap_index+i);
  }
  void* addr=(void*)(bmap_index<<12);
  return addr;
}

void pmem_free(void* start,int num_pages) {
  int start_page=(size_t)start>>FRAME_NO_OFFSET;
  for (int i=start_page;i<num_pages;i++) {
    set_bmap_bit(i);
  }
}
