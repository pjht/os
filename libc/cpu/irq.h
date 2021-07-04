#ifndef IRQ_H
#define IRQ_H

/**
 * Registers the current process as an interrupt handler
 * \param int_no The interrupt to register for
 * \param handler The address of the handler to register
*/
void register_irq_handler(int irq_no,void* handler);

#endif
