extern main
extern __stdio_init
global _start

_start:
  call __stdio_init
  call main
  ret
