extern main
extern exit
extern __stdio_init
global _start

_start:
  call __stdio_init
  call main
  push 0
  call exit
  ret
