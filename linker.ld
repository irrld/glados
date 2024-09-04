ENTRY(_start)
SECTIONS
{
  . = 1M;
    /* Place Multiboot header at the very beginning of the kernel */
    .boot :
    {
        *(.multiboot_header) /* Ensures that this section is not discarded */
    }

    .text : { *(.text); }
    .rodata : { *(.rodata); }

    .data : { *(.data); }

    .bss :
    {
        *(.bss)
        *(COMMON)
    }

    .stack (NOLOAD) :
    {
        . = ALIGN(16);
        __stack_start = .;
        . = . + 0x8000;  /* Reserve 32KB for stack */
        __stack_end = .;
    }
    __image_end = .;

    /DISCARD/ : { *(.comment) *(.eh_frame) *(.eh_frame_hdr) }
}