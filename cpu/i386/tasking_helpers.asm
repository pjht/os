global set_regs
set_regs:
mov eax, [esp+4]
mov [esp_strg], eax
mov eax, [esp+8]
mov [ebp_strg], eax
mov eax, [esp+12]
mov [eip_strg], eax
mov esp, [esp_strg]
mov ebp, [ebp_strg]
mov eax, 0x12345
jmp [eip_strg]

esp_strg:resd 1
ebp_strg:resd 1
eip_strg:resd 1

global read_eip
read_eip:
  pop eax
  jmp eax
