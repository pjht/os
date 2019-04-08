#ifndef INT_TASKING_H
#define INT_TASKING_H

#include <stdint.h>

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} Registers;

typedef struct Task {
    Registers regs;
    struct Task* next;
    char kmode;
    char** msg_store;
    uint32_t* sender_store;
    uint32_t msg_indx;
    uint8_t rd;
    uint8_t wr;
    uint32_t pid;
    char priv;
    int errno;
} Task;

int* tasking_get_errno_address();

#endif
