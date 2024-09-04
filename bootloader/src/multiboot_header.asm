section .multiboot_header
multiboot_header_start:
    dd 0xE85250D6                ; magic number
    dd 0                         ; protected mode code
    dd multiboot_header_end - multiboot_header_start ; header length
    ; checksum
    dd 0x100000000 - (0xE85250D6 + 0 + (multiboot_header_end - multiboot_header_start))

    dw 5
    dw 0
    dd 8
    dw 5
    dw 0
    dd 8
multiboot_header_end:

