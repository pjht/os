section .text
global switch_to_task
extern currentTask
extern tss
;WARNING: Caller is expected to disable IRQs before calling, and enable IRQs again after function returns

switch_to_task:

    ;Save previous task's state

    ;Notes:
    ;  For cdecl; EAX, ECX, and EDX are already saved by the caller and don't need to be saved again
    ;  EIP is already saved on the stack by the caller's "CALL" instruction
    ;  The task isn't able to change CR3 so it doesn't need to be saved
    ;  Segment registers are constants (while running kernel code) so they don't need to be saved

    push ebx
    push esi
    push edi
    push ebp

    mov edi,[currentTask]    ;edi = address of the previous task's data structure
    mov [edi],esp         ;Save ESP for the task's kernel stack in the task's data structure

    ;Load next task's state

    mov esi,[esp+(4+1)*4]         ;esi = address of the next task's data structure
    mov [currentTask],esi    ;Current task's task data is the next task thread data

    mov esp,[esi]         ;Load ESP for next task's kernel stack from the task's data structure
    mov eax,[esi+8]         ;eax = address of page directory for next task
    mov ebx,[esi+4]        ;ebx = address for the top of the next task's kernel stack
    mov [tss+4],ebx            ;Adjust the ESP0 field in the TSS (used by CPU for for CPL=3 -> CPL=0 privilege level changes)
    mov ecx,cr3                   ;ecx = previous task's virtual address space

    cmp eax,ecx                   ;Does the virtual address space need to being changed?
    je .doneVAS                   ; no, virtual address space is the same, so don't reload it and cause TLB flushes
    mov cr3,eax                   ; yes, load the next task's virtual address space
.doneVAS:

    pop ebp
    pop edi
    pop esi
    pop ebx

    ret                           ;Load next task's EIP from its kernel stack

global task_init

task_init:
pop ecx
pop ebx
cli
mov ax, 0x23
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax

push 0x23
push ebx
pushf
pop eax

or eax, 0x200
push eax
push 0x1B
push ecx
iret
