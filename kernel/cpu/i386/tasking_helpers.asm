section .text
global switch_to_thread_asm
extern load_address_space
extern currentThread
extern tss
;WARNING: Caller is expected to disable IRQs before calling, and enable IRQs again after function returns

switch_to_thread_asm:

    ;Save previous thread's state

    ;Notes:
    ;  For cdecl; EAX, ECX, and EDX are already saved by the caller and don't need to be saved again
    ;  EIP is already saved on the stack by the caller's "CALL" instruction
    ;  The thread isn't able to change CR3 so it doesn't need to be saved
    ;  Segment registers are constants (while running kernel code) so they don't need to be saved

    push ebx
    push esi
    push edi
    push ebp

    mov edi,[currentThread]    ;edi = address of the previous thread's data structure
    mov [edi],esp         ;Save ESP for the thread's kernel stack in the thread's data structure

    ;Load next thread's state

    mov esi,[esp+(4+1)*4]         ;esi = address of the next thread's data structure
    mov [currentThread],esi    ;Set the current thread to the thread we are switching to

    mov esp,[esi]         ;Load ESP for next thread's kernel stack from the thread's data structure
    mov eax,[esi+8]         ;eax = address of page directory for next thread
    mov ebx,[esi+4]        ;ebx = address for the top of the next thread's kernel stack
    mov [tss+4],ebx            ;Adjust the ESP0 field in the TSS (used by CPU for for CPL=3 -> CPL=0 privilege level changes)
    mov ecx,cr3                   ;ecx = previous thread's virtual address space

    cmp eax,ecx                   ;Does the virtual address space need to being changed?
    je .doneVAS                   ; no, virtual address space is the same, so don't reload it and cause TLB flushes
    ; yes, load the next thread's virtual address space
    push eax
    call load_address_space
    add esp,4
.doneVAS:
    pop ebp
    pop edi
    pop esi
    pop ebx

    ret                           ;Load next thread's EIP from its kernel stack

global task_init

; Switch to usermode, given a usermode stack and EIP to switch to

task_init:
  pop ecx ; ecx = user ESP
  pop ebx ; ebx = user EIP
  mov ax, 0x23 ; Load data segment selectors with the usermode data segment
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  push 0x23 ; Push SS
  push ecx ; push user ESP
  pushf ; push flags
  pop eax ; pop flags into eax
  or eax, 0x200 ; enable interrupts when iret runs 
  push eax ; push modified flags
  push 0x1B ; push CS
  push ebx ; push user EIP
  iret

global wait_for_unblocked_thread_asm

wait_for_unblocked_thread_asm:
  sti ;As interrupts are stopped in tasking code, re-enable them
  hlt ;Wait for an interrupt handler to run and return.
  cli ;Clear interrupts, as tasking code must not be run with interrupts on.
