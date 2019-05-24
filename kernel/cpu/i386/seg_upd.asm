global seg_upd
seg_upd:
jmp 0x8:code_upd
code_upd:
mov ax, 0x10
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax
ret
