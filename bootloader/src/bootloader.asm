section .bss
global multiboot_info_addr_
multiboot_info_addr_:
     resb 4

section .text
    [bits 32]

global _start
_start:
    mov [multiboot_info_addr_], ebx
    mov ebx, prot_mode_msg
    call print_string32

    ;; Build 4 level page table and switch to long mode
    mov ebx, 0x1000
    call build_page_table
    mov cr3, ebx            ; MMU finds the PML4 table in cr3

    ;; Enable Physical Address Extension (PAE)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ;; The EFER (Extended Feature Enable Register) MSR (Model-Specific Register) contains fields
    ;; related to IA-32e mode operation. Bit 8 if this MSR is the LME (long mode enable) flag
    ;; that enables IA-32e operation.
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ;; Enable paging (PG flag in cr0, bit 31)
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ;; New GDT has the 64-bit segment flag set. This makes the CPU switch from
    ;; IA-32e compatibility mode to 64-bit mode.
    lgdt [gdt64_pseudo_descriptor]

    mov ax, DATA_SEG64
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    jmp CODE_SEG64:start_long_mode

end:
    hlt
    jmp end

%include "include/print32.asm"

    ;; Builds a 4 level page table starting at the address that's passed in ebx.
build_page_table:
    pusha

    PAGE64_PAGE_SIZE equ 0x1000
    PAGE64_TAB_SIZE equ 0x1000
    PAGE64_TAB_ENT_NUM equ 512

    ;; Initialize all four tables to 0. If the present flag is cleared, all other bits in any
    ;; entry are ignored. So by filling all entries with zeros, they are all "not present".
    ;; Each repetition zeros four bytes at once. That's why a number of repetitions equal to
    ;; the size of a single page table is enough to zero all four tables.
    mov ecx, PAGE64_TAB_SIZE ; ecx stores the number of repetitions
    mov edi, ebx             ; edi stores the base address
    xor eax, eax             ; eax stores the value
    rep stosd

    ;; Link first entry in PML4 table to the PDP table
    mov edi, ebx
    lea eax, [edi + (PAGE64_TAB_SIZE | 11b)] ; Set read/write and present flags
    mov dword [edi], eax

    ;; Link first entry in PDP table to the PD table
    add edi, PAGE64_TAB_SIZE
    add eax, PAGE64_TAB_SIZE
    mov dword [edi], eax

    ;; Link the first entry in the PD table to the page table
    add edi, PAGE64_TAB_SIZE
    add eax, PAGE64_TAB_SIZE
    mov dword [edi], eax

    ;; Identity map the first 2 MB of memory in the single page table
    add edi, PAGE64_TAB_SIZE
    mov ebx, 11b
    mov ecx, PAGE64_TAB_ENT_NUM

build_page_table_set_entry:
    mov dword [edi], ebx
    add ebx, PAGE64_PAGE_SIZE
    add edi, 8
    loop build_page_table_set_entry

    popa
    ret

    [bits 64]

start_long_mode:
    mov ebx, long_mode_msg
    call print_string64

    extern __stack_end
    mov rsp, __stack_end

    extern __image_end
    mov rdi, __image_end

    extern kmain
    call kmain

end64:
    hlt
    jmp end64

%include "include/print64.asm"

%include "include/gdt64.asm"

stage2_msg: db "Disk load successful", 13, 10, 0
mem_failed_msg: db "Failed to get memory", 13, 10, 0
prot_mode_msg: db "Switched to protected mode", 0
long_mode_msg: db "Switched to 64-bit long mode", 0