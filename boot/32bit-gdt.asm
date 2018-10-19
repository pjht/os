gdt_start: ; don't remove the labels, they're needed to compute sizes and jumps
    ; the GDT starts with a null 8-byte
    dd 0x0 ; 4 byte
    dd 0x0 ; 4 byte

; GDT for kernel code segment. base = 0x00000000, length = 0xfffff
; for flags, refer to os-dev.pdf document, page 36
gdt_kern_code:
    dw 0x0000    ; segment length, bits 0-15
    dw 0x0       ; segment base, bits 0-15
    db 0x0       ; segment base, bits 16-23
    db 10011010b ; flags (8 bits)
    db 11000001b ; flags (4 bits) + segment length, bits 16-19
    db 0x0       ; segment base, bits 24-31

; GDT for kernel data segment. base and length identical to code segment
; some flags changed, again, refer to os-dev.pdf
gdt_kern_data:
    dw 0x0000
    dw 0x0
    db 0x0
    db 10010010b
    db 11000001b
    db 0x0

; GDT for user code segment. base = 0x01000000, length = 0xfffff
; for flags, refer to os-dev.pdf document, page 36
gdt_usr_code:
    dw 0xffff    ; segment length, bits 0-15
    dw 0x0      ; segment base, bits 0-15
    db 0x0       ; segment base, bits 16-23
    db 11111010b ; flags (8 bits)
    db 11001111b ; flags (4 bits) + segment length, bits 16-19
    db 0x0      ; segment base, bits 24-31

; GDT for user data segment. base and length identical to code segment
; some flags changed, again, refer to os-dev.pdf
gdt_usr_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 11110010b
    db 11001111b
    db 0x0

gdt_end:

; GDT descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; size (16 bit), always one less of its true size
    dd gdt_start ; address (32 bit)

; define some constants for later use
CODE_SEG equ gdt_kern_code - gdt_start
DATA_SEG equ gdt_kern_data - gdt_start
USR_CODE_SEG equ gdt_usr_code - gdt_start
USR_DATA_SEG equ gdt_usr_data - gdt_start
