;
;
;

[org 0x7c00] ; This tells assembler where we're going to be in memory
KERNEL_OFFSET equ 0x1000

    mov [BOOT_DRIVE], dl

    mov bp, 0x8000 ; set our stack, somewhere safe
    mov sp, bp

    mov bx, MSG_REAL_MODE
    call print_string

    call load_kernel

    call switch_to_pm

    jmp $


%include "print.asm"
%include "disk/disk_load.asm"
%include "pm/gdt.asm"
%include "pm/print_pm.asm"
%include "pm/switch_to_pm.asm"

[bits 16]

load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call print_string

    mov bx, KERNEL_OFFSET
    mov dh , 15             ; load 5 sectors from disk
    mov dl, [BOOT_DRIVE]
    call disk_load

    ret

[bits 32]

begin_pm:

    mov ebx, MSG_PROT_MODE
    call print_string_pm

    call KERNEL_OFFSET ; Here we go to our kernel code!

    jmp $

; Global variables
BOOT_DRIVE: db 0
MSG_REAL_MODE db "Started in 16-bit Real Mode", 0x0D, 0x0A, 0
MSG_PROT_MODE db "Successfully landed in 32-bit Protected Mode", 0x0D, 0x0A, 0
MSG_LOAD_KERNEL db "Loading kernel...", 0x0D, 0x0A, 0

; Boot sector magic
times 510-($-$$) db 0

dw 0xaa55
