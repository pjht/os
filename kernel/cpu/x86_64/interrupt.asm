; Defined in isr.c
[extern isr_handler]
[extern irq_handler]

; Common ISR code
isr_common_stub:
    ; 1. Save CPU state
	push rax
	push rcx
	push rdx
	push rbx
	push rsp
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
  ; 2. Call C handler
	mov rdi,rsp
	call isr_handler
  ; 3. Restore state
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rsp
	pop rbx
	pop rdx
	pop rcx
	pop rax
	add esp, 16 ; Cleans up the pushed error code and pushed ISR number
	sti
	iretq ; pops 5 things at once: CS, EIP, RFLAGS, SS, and RSP

; Common IRQ code. Identical to ISR code except for the 'call'
; and the 'pop ebx'
irq_common_stub:
	; 1. Save CPU state
	push rax
	push rcx
	push rdx
	push rbx
	push rsp
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	; 2. Call C handler
	mov rdi,rsp
	call irq_handler
	; 3. Restore state
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rsp
	pop rbx
	pop rdx
	pop rcx
	pop rax
	add esp, 16 ; Cleans up the pushed error code and pushed ISR number
	sti
	iretq ; pops 5 things at once: CS, EIP, RFLAGS, SS, and RSP

; We don't get information about which interrupt was caller
; when the handler is run, so we will need to have a different handler
; for every interrupt.
; Furthermore, some interrupts push an error code onto the stack but others
; don't, so we will push a dummy error code for those which don't, so that
; we have a consistent stack for all of them.

; First make the ISRs global
global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31
global isr80
; IRQs
global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

; 0: Divide By Zero Exception
isr0:
    cli
    push qword 0
    push qword 0
    jmp isr_common_stub

; 1: Debug Exception
isr1:
    cli
    push qword 0
    push qword 1
    jmp isr_common_stub

; 2: Non Maskable Interrupt Exception
isr2:
    cli
    push qword 0
    push qword 2
    jmp isr_common_stub

; 3: Int 3 Exception
isr3:
    cli
    push qword 0
    push qword 3
    jmp isr_common_stub

; 4: INTO Exception
isr4:
    cli
    push qword 0
    push qword 4
    jmp isr_common_stub

; 5: Out of Bounds Exception
isr5:
    cli
    push qword 0
    push qword 5
    jmp isr_common_stub

; 6: Invalid Opcode Exception
isr6:
    cli
    push qword 0
    push qword 6
    jmp isr_common_stub

; 7: Coprocessor Not Available Exception
isr7:
    cli
    push qword 0
    push qword 7
    jmp isr_common_stub

; 8: Double Fault Exception (With Error Code!)
isr8:
    cli
    push qword 8
    jmp isr_common_stub

; 9: Coprocessor Segment Overrun Exception
isr9:
    cli
    push qword 0
    push qword 9
    jmp isr_common_stub

; 10: Bad TSS Exception (With Error Code!)
isr10:
    cli
    push qword 10
    jmp isr_common_stub

; 11: Segment Not Present Exception (With Error Code!)
isr11:
    cli
    push qword 11
    jmp isr_common_stub

; 12: Stack Fault Exception (With Error Code!)
isr12:
    cli
    push qword 12
    jmp isr_common_stub

; 13: General Protection Fault Exception (With Error Code!)
isr13:
    cli
    push qword 13
    jmp isr_common_stub

; 14: Page Fault Exception (With Error Code!)
isr14:
    cli
    push qword 14
    jmp isr_common_stub

; 15: Reserved Exception
isr15:
    cli
    push qword 0
    push qword 15
    jmp isr_common_stub

; 16: Floating Point Exception
isr16:
    cli
    push qword 0
    push qword 16
    jmp isr_common_stub

; 17: Alignment Check Exception
isr17:
    cli
    push qword 0
    push qword 17
    jmp isr_common_stub

; 18: Machine Check Exception
isr18:
    cli
    push qword 0
    push qword 18
    jmp isr_common_stub

; 19: Reserved
isr19:
    cli
    push qword 0
    push qword 19
    jmp isr_common_stub

; 20: Reserved
isr20:
    cli
    push qword 0
    push qword 20
    jmp isr_common_stub

; 21: Reserved
isr21:
    cli
    push qword 0
    push qword 21
    jmp isr_common_stub

; 22: Reserved
isr22:
    cli
    push qword 0
    push qword 22
    jmp isr_common_stub

; 23: Reserved
isr23:
    cli
    push qword 0
    push qword 23
    jmp isr_common_stub

; 24: Reserved
isr24:
    cli
    push qword 0
    push qword 24
    jmp isr_common_stub

; 25: Reserved
isr25:
    cli
    push qword 0
    push qword 25
    jmp isr_common_stub

; 26: Reserved
isr26:
    cli
    push qword 0
    push qword 26
    jmp isr_common_stub

; 27: Reserved
isr27:
    cli
    push qword 0
    push qword 27
    jmp isr_common_stub

; 28: Reserved
isr28:
    cli
    push qword 0
    push qword 28
    jmp isr_common_stub

; 29: Reserved
isr29:
    cli
    push qword 0
    push qword 29
    jmp isr_common_stub

; 30: Reserved
isr30:
    cli
    push qword 0
    push qword 30
    jmp isr_common_stub

; 31: Reserved
isr31:
    cli
    push qword 0
    push qword 31
    jmp isr_common_stub

; 80: Syscalls
isr80:
    cli
    push qword 0
    push qword 80
    jmp isr_common_stub

; IRQ handlers
irq0:
	cli
	push qword 0
	push qword 32
	jmp irq_common_stub

irq1:
	cli
	push qword 1
	push qword 33
	jmp irq_common_stub

irq2:
	cli
	push qword 2
	push qword 34
	jmp irq_common_stub

irq3:
	cli
	push qword 3
	push qword 35
	jmp irq_common_stub

irq4:
	cli
	push qword 4
	push qword 36
	jmp irq_common_stub

irq5:
	cli
	push qword 5
	push qword 37
	jmp irq_common_stub

irq6:
	cli
	push qword 6
	push qword 38
	jmp irq_common_stub

irq7:
	cli
	push qword 7
	push qword 39
	jmp irq_common_stub

irq8:
	cli
	push qword 8
	push qword 40
	jmp irq_common_stub

irq9:
	cli
	push qword 9
	push qword 41
	jmp irq_common_stub

irq10:
	cli
	push qword 10
	push qword 42
	jmp irq_common_stub

irq11:
	cli
	push qword 11
	push qword 43
	jmp irq_common_stub

irq12:
	cli
	push qword 12
	push qword 44
	jmp irq_common_stub

irq13:
	cli
	push qword 13
	push qword 45
	jmp irq_common_stub

irq14:
	cli
	push qword 14
	push qword 46
	jmp irq_common_stub

irq15:
	cli
	push qword 15
	push qword 47
	jmp irq_common_stub
