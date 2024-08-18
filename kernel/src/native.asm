global idt_load
extern idtp_
idt_load:
    lidt [idtp_]    ; load the IDT pointer using lidt instruction
    ret

extern isr_common_stub

%macro ISR_STUB 1
    global isr_%1
isr_%1:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, %1  ; interrupt number is passed to the first parameter of the isr_common_stub function

    call isr_common_stub

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    iretq
%endmacro

ISR_STUB 33

global enable_interrupts
enable_interrupts:
    sti ; set interrupt flag
    ret

global disable_interrupts
disable_interrupts:
    cli ; clear interrupt flag
    ret

global halt
halt:
    hlt
    ret