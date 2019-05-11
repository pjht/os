.section .text
.global switchTask
switchTask:
  pop %eax
  pop %eax
  mov %eax,%esp
  pop %eax
  mov %ax,%ds
  mov %ax,%es
  mov %ax,%fs
  mov %ax,%gs
  popa
  add $8,%esp # Cleans up the pushed error code and pushed ISR number
  iret # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP
