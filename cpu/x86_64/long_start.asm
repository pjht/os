global long_mode_start
extern kmain

section .text
bits 64
long_mode_start:
    mov rax,kmain+0xffff800000000000
    call rax
    loop: jmp loop
