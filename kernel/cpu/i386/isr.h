/**
 * \file 
*/

#ifndef ISR_H
#define ISR_H

#include <stdint.h>
#include <sys/types.h>

/** 
 * Saved state of the CPU when an interrupt occurs
*/
typedef struct {
   uint32_t ds; //!< Data segment selector
   uint32_t edi; //!< Pushed by pusha.
   uint32_t esi; //!< Pushed by pusha.
   uint32_t ebp; //!< Pushed by pusha.
   uint32_t esp; //!< Pushed by pusha.
   uint32_t ebx; //!< Pushed by pusha.
   uint32_t edx; //!< Pushed by pusha.
   uint32_t ecx; //!< Pushed by pusha.
   uint32_t eax; //!< Pushed by pusha.
   uint32_t int_no; //!< Interrupt number
   uint32_t err_code; //!< Error code (if applicable)
   uint32_t eip; //!< Pushed by the processor automatically
   uint32_t cs; //!< Pushed by the processor automatically
   uint32_t eflags; //!< Pushed by the processor automatically
   uint32_t useresp; //!< Pushed by the processor automatically
   uint32_t ss; //!< Pushed by the processor automatically
} registers_t;

typedef void (*isr_t)(registers_t*); //!< Type of an ISR handler function pointer

/**
 * Install the interrupt handlers into the IDT.
*/
void isr_install();

/**
 * Register an IRQ handler
 * 
 * If the PID is 0, the handler will be called directly, otherwise a thread will be made in the PID starting at the handler's address.
 * \param n The IRQ to register a handler for
 * \param pid The PID that will handle the interrupt. 
 * \param handler The address of the handler.
*/
void isr_register_handler(int n,pid_t pid,void* handler);

#endif
