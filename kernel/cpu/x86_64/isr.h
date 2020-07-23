#ifndef ISR_H
#define ISR_H

#include <stdint.h>

/* Struct which aggregates many registers */
typedef struct {
   uint64_t r15,r14,r13,r12,r11,r10,r9,r8,rdi,rsi,rbp,rsp,rbx,rdx,rcx,rax; /* Pushed by our code. */
   uint64_t int_no,err_code; /* Interrupt number and error code (if applicable) */
   uint64_t rip,cs,rflags,userrsp,ss; /* Pushed by the processor automatically */
} registers_t;

typedef void (*isr_t)(registers_t*);

void isr_install();
void isr_register_handler(int n,isr_t handler);

#endif
