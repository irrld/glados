[bits 64]
[extern _start_kernel]
    call _start_kernel
    jmp $ ; Hang forever when we return from the kernel