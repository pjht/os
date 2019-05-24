global long_mode_start
extern kmain

section .boot.text
bits 64
long_mode_start:
    xchg bx,bx
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov rax,kmain
    call rax
    loop: jmp loop
