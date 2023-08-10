print_char:
    push ax
    mov ah, 0x0e ; tty mode
    mov al, bl ; get char
    int 0x10 ; print char
    pop ax
    ret

; prints bx
print_string:
    push ax
    push bx
    mov ah, 0x0e ; tty mode
    mov al, [bx] ; get char
    print_internal:
    int 0x10 ; print char
    inc bx ; next char
    mov al, [bx] ; get next char
    cmp al, 0x0 ; check for null terminator
    jne print_internal ; if not, print next char
    pop bx
    pop ax
    ret

; shifts bx by cx
shr_by_cx:
    cmp cx, 0
    je skip_shift
    push ax
    mov ax, cx
    shift:
    shr bx, 4
    dec ax
    cmp ax, 0
    jne shift
    pop ax
    skip_shift:
    ret

; prints the value of dx as hex.
print_hex:
    pusha
    mov bx, HEX_PREFIX
    call print_string
    mov ax, 0xf000 ; mask
    mov cx, 3
    loop1:
    mov bx, dx
    and bx, ax
    call shr_by_cx
    dec cx
    mov bh, 0x00
    cmp bl, 9
    jle inc_to_num
    add bl, 0x27
    inc_to_num:
    add bl, 0x30
    call print_char
    ;
    shr ax, 4
    cmp ax, 0x0 ; check loop
    jne loop1
    popa
    ret

; global variables
HEX_PREFIX: db '0x', 0x0
NEWLINE: db 0x0A, 0x0D, 0x0