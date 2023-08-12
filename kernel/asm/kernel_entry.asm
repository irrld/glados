[bits 64]
[extern KernelMain]
    call KernelMain
    jmp $ ; Hang forever when we return from the kernel