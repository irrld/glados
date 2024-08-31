extern idtp_
extern isr_cpu_state_

extern send_eoi

global idt_load
idt_load:
    lidt [idtp_]    ; load the IDT pointer using lidt instruction
    ret

%macro ISR_STUB_COMMON 1
extern irh_%1
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
    call irh_%1
    mov rdi, %1  ; interrupt number is passed to the first parameter of the isr_common_stub function
    call send_eoi

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

; ISR stub that saves the cpu state to a variable instead of the stack
%macro ISR_STUB_SWITCHING 1
extern irh_%1
global isr_%1
isr_%1:
; Save cpu state
    push rdi

    mov rdi, isr_cpu_state_

    mov [rdi + 64], r8
    mov [rdi + 72], r9
    mov [rdi + 80], r10
    mov [rdi + 88], r11
    mov [rdi + 96], r12
    mov [rdi + 104], r13
    mov [rdi + 112], r14
    mov [rdi + 120], r15
    mov [rdi + 0], rax
    mov [rdi + 8], rbx
    mov [rdi + 16], rcx
    mov [rdi + 24], rdx
    mov [rdi + 32], rbp
    mov [rdi + 48], rsi
    pop rsi ; Restore rdi to rsi
    mov [rdi + 56], rsi ; Write rdi (on rsi)

    ; Pop RIP, CS and RFLAGS to r15, r14 and r13..
    pop r15 ; RIP
    pop r14 ; CS
    pop r13 ; RFLAGS
    pop r12 ; RSP
    pop r11 ; SS

    ; Write r15, r14 and r13 to their respected locations in rdi (cpu_state struct)
    mov [rdi + 128], r15 ; RIP
    mov [rdi + 144], r14 ; CS
    mov [rdi + 136], r13 ; RFLAGS
    mov [rdi + 40], r12 ; RSP
    mov [rdi + 152], r11 ; SS

    mov rsp, [rdi + 40]
    mov rbp, [rdi + 32]

    mov rdi, %1  ; interrupt number is passed to the first parameter of the isr_common_stub function
    call irh_%1
    mov rdi, %1  ; interrupt number is passed to the first parameter of the isr_common_stub function
    call send_eoi

    ; Load cpu state
    mov rdi, isr_cpu_state_

    mov rax, [rdi + 0]
    mov rbx, [rdi + 8]
    mov rcx, [rdi + 16]
    mov rdx, [rdi + 24]
    mov rbp, [rdi + 32]
    mov rsi, [rdi + 48]

    mov rsp, [rdi + 40] ; Restore RSP before pushing values back

    mov r8, 0x00      ; Unknown
    push r8                  ; Push unknown value??
    mov r8, [rdi + 152]      ; SS
    push r8                  ; Push SS
    mov r8, [rdi + 40]       ; RSP
    push r8                  ; Push RSP
    mov r8, [rdi + 136]      ; RFLAGS
    push r8                  ; Push RFLAGS
    mov r8, [rdi + 144]      ; CS
    push r8                  ; Push CS
    mov r8, [rdi + 128]      ; RIP
    push r8                  ; Push RIP

    mov r8, [rdi + 64]
    mov r9, [rdi + 72]
    mov r10, [rdi + 80]
    mov r11, [rdi + 88]
    mov r12, [rdi + 96]
    mov r13, [rdi + 104]
    mov r14, [rdi + 112]
    mov r15, [rdi + 120]

    mov rdi, [rdi + 56] ; Finally, load rdi

    iretq
%endmacro

ISR_STUB_SWITCHING 0
ISR_STUB_SWITCHING 13
ISR_STUB_SWITCHING 14
ISR_STUB_SWITCHING 32
ISR_STUB_COMMON 33
ISR_STUB_COMMON 40

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