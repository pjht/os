#include "cpu/arch_consts.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define KMALLOC_BMAP_SZ (((KMALLOC_SZ*1024)/4)/8)

static char bitmap[KMALLOC_BMAP_SZ];
static void* data=(void*)KMALLOC_START;


static char get_bmap_bit(size_t index) {
  size_t byte=index/8;
  size_t bit=index%8;
  char entry=bitmap[byte];
  return (entry&(1<<bit))>0;
}

static void set_bmap_bit(size_t index) {
  size_t byte=index/8;
  size_t bit=index%8;
  bitmap[byte]=bitmap[byte]|(1<<bit);
}

static void clear_bmap_bit(size_t index) {
  size_t byte=index/8;
  size_t bit=index%8;
  bitmap[byte]=bitmap[byte]&(~(1<<bit));
}

void* kmalloc(size_t size) {
  size_t num_4b_grps=(size_t)ceilf((float)size/4);
  num_4b_grps+=2;
  size_t bmap_index;
  size_t remaining_blks;
  for(size_t i=0;i<KMALLOC_BMAP_SZ;i++) {
    char got_0=0;
    remaining_blks=num_4b_grps;
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
  for (size_t i=0;i<num_4b_grps;i++) {
    set_bmap_bit(bmap_index+i);
  }
  size_t data_offset=(bmap_index*8)+8;
  size_t* info=(void*)(((char*)data)+data_offset-8);
  info[0]=num_4b_grps;
  info[1]=bmap_index;
  return (void*)(((char*)data)+data_offset);

}

void kfree(void* mem) {
  size_t* info=(size_t*)((size_t)mem-8);
  size_t num_4b_grps=info[0];
  size_t bmap_index=info[1];
  for (size_t i=0;i<num_4b_grps;i++) {
    clear_bmap_bit(bmap_index+i);
  }
}
