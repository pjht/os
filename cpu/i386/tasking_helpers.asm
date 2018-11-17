global set_regs
set_regs:
; movw [esp_strg], [esp+28]
; movw [ebp_strg], [esp+32]
; movw [eip_strg], [esp+36]
mov eax, [esp+40]
pushf
popf
mov eax, [esp+4]
mov ebx, [esp+8]
mov ecx, [esp+12]
mov edx, [esp+16]
mov esi, [esp+20]
mov edi, [esp+24]
mov esp, [esp_strg]
mov ebp, [ebp_strg]
jmp [eip_strg]

esp_strg:resd 1
ebp_strg:resd 1
eip_strg:resd 1
; eflags_strg:resd 1
