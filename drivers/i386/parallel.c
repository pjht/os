#include <stdint.h>
#include "../../kernel/klog.h"

uint16_t* bda_lpt=(uint16_t*)0xc0000408;

int parallel_data_port(int lpt) {
  return bda_lpt[lpt];
}

int parallel_stat_port(int lpt) {
  return bda_lpt[lpt]+1;
}

int parallel_cont_port(int lpt) {
  return bda_lpt[lpt]+2;
}

void parallel_init() {
  klog("INFO","Scanning for parallel ports");
  for (int i=0;i<3;i++) {
    if (parallel_data_port(i)>0) {
      klog("INFO","Found LPT%d",i+1);
    }
  }
}
