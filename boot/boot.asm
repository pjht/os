[org 0x7c00]
KERNEL_OFFSET equ 0x1000 ; The same one we used when linking the kernel

    mov [BOOT_DRIVE], dl ; Remember that the BIOS sets us the boot drive in 'dl' on boot
    mov bp, 0x9000
    mov sp, bp

    mov     ax,2403h                ;--- A20-Gate Support ---
    int     15h
    jb      a20_failed                 ;INT 15h is not supported
    cmp     ah,0
    jnz     a20_failed                  ;INT 15h is not supported

    mov     ax,2402h                ;--- A20-Gate Status ---
    int     15h
    jb      a20_failed              ;couldn't get status
    cmp     ah,0
    jnz     a20_failed              ;couldn't get status

    cmp     al,1
    jz      a20_activated           ;A20 is already activated

    mov     ax,2401h                ;--- A20-Gate Activate ---
    int     15h
    jb      a20_failed              ;couldn't activate the gate
    cmp     ah,0
    jnz     a20_failed              ;couldn't activate the gate

    a20_activated:                  ;go on
    call load_kernel ; read the kernel from disk
    call switch_to_pm ; disable interrupts, load GDT,  etc. Finally jumps to 'BEGIN_PM'
    jmp $ ; Never executed

%include "boot/print.asm"
%include "boot/print_hex.asm"
%include "boot/disk.asm"
%include "boot/32bit-gdt.asm"
%include "boot/32bit-switch.asm"

[bits 16]
load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call print
    call print_nl
    mov bx, KERNEL_OFFSET ; Read from disk and store in 0x1000
    mov dh, 50
    mov dl, [BOOT_DRIVE]
    call disk_load
    ret

a20_failed:
  mov bx, MSG_NO_A20
  call print
  call print_nl

[bits 32]
BEGIN_PM:
    call KERNEL_OFFSET ; Give control to the kernel
    jmp $


BOOT_DRIVE db 0 ; It is a good idea to store it in memory because 'dl' may get overwritten
MSG_LOAD_KERNEL db "Loading kernel into memory", 0
MSG_NO_A20 db "No A20 line! Halting",0
; padding
times 510 - ($-$$) db 0
dw 0xaa55
