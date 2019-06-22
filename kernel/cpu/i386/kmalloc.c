#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

static char bitmap[2097152];
static void* data=(void*)0xFE800000;


static char get_bmap_bit(uint32_t index) {
  uint32_t byte=index/8;
  uint32_t bit=index%8;
  char entry=bitmap[byte];
  return (entry&(1<<bit))>0;
}

static void set_bmap_bit(uint32_t index) {
  uint32_t byte=index/8;
  uint32_t bit=index%8;
  bitmap[byte]=bitmap[byte]|(1<<bit);
}

static void clear_bmap_bit(uint32_t index) {
  uint32_t byte=index/8;
  uint32_t bit=index%8;
  bitmap[byte]=bitmap[byte]&(~(1<<bit));
}

void* kmalloc(uint32_t size) {
  uint32_t num_4b_grps=(uint32_t)ceilf((float)size/4);
  num_4b_grps+=2;
  uint32_t bmap_index;
  uint32_t remaining_blks;
  for(uint32_t i=0;i<2097152;i++) {
    if (bitmap[i]!=0xFF) {
      char got_0=0;
      remaining_blks=num_4b_grps;
      uint32_t old_j;
      for (uint32_t j=i*8;;j++) {
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
    }
    if (remaining_blks==0) {
      break;
    }
  }
  if (remaining_blks!=0) {
    return NULL;
  }
  for (uint32_t i=0;i<num_4b_grps;i++) {
    set_bmap_bit(bmap_index+i);
  }
  uint32_t data_offset=(bmap_index*8)+8;
  uint32_t* info=(void*)(((char*)data)+data_offset-8);
  info[0]=num_4b_grps;
  info[1]=bmap_index;
  return (void*)(((char*)data)+data_offset);

}

void kfree(void* mem) {
  uint32_t* info=(uint32_t*)((uint32_t)mem-12);
  uint32_t num_4b_grps=info[0];
  uint32_t bmap_index=info[1];
  for (uint32_t i=0;i<num_4b_grps;i++) {
    clear_bmap_bit(bmap_index+i);
  }
}
