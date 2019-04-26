global long_mode_start
extern kmain

section .boot.text
bits 64
long_mode_start:
    mov rax,kmain
    call rax
    loop: jmp loop
