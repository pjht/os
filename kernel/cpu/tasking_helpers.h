#ifndef TASKING_HELPERS_H
#define TASKING_HELPERS_H

#include "../tasking.h"

void switch_to_thread_asm(Thread* thread);
void task_init();
void wait_for_unblocked_thread_asm();
void setup_kstack(Thread* thread,void* param1,void* param2,char kmode,void* eip);

#endif
