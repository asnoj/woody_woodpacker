BITS 64
section .text
global _start

_start:
    ; --- sauvegarde de TOUT l'etat ---
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rdi
    push rsi
    push rbp
    push rdx
    push rcx
    push rbx
    push rax

    ; --- 1. afficher "....WOODY....\n" ---
    lea  rsi, [rel msg]
    mov  rdx, msg_len
    mov  rdi, 1
    mov  rax, 1
    syscall

    ; --- 2. adresse reelle du code ---
    lea  rbx, [rel _start]
    mov  r12, [rel code_delta]
    add  r12, rbx                 ; r12 = adresse du code en memoire

    ; --- 3. mprotect(page, len+marge, RWX) ---
    mov  rdi, r12
    and  rdi, ~0xFFF
    mov  rsi, [rel code_len]
    add  rsi, 0x1000
    mov  rdx, 7
    mov  rax, 10
    syscall

    ; --- 4. espace pour S sur la pile ---
    sub  rsp, 256
    mov  r13, rsp

    ; --- 5. KSA ---
    xor  rcx, rcx
.ksa_init:
    mov  [r13 + rcx], cl
    inc  rcx
    cmp  rcx, 256
    jb   .ksa_init

    lea  rsi, [rel key]
    xor  rcx, rcx
    xor  rbx, rbx
.ksa_mix:
    movzx rax, byte [r13 + rcx]
    add  bl, al
    mov  rdx, rcx
    and  edx, 0x0f
    add  bl, [rsi + rdx]
    movzx rdx, byte [r13 + rbx]
    mov  [r13 + rcx], dl
    mov  [r13 + rbx], al
    inc  rcx
    cmp  rcx, 256
    jb   .ksa_mix

    ; --- 6. PRGA + XOR (dechiffrement) ---
    xor  rcx, rcx
    xor  rbx, rbx
    xor  r14, r14
    mov  r15, [rel code_len]
.prga:
    inc  cl
    movzx rax, byte [r13 + rcx]
    add  bl, al
    movzx rdx, byte [r13 + rbx]
    mov  [r13 + rcx], dl
    mov  [r13 + rbx], al
    movzx rax, byte [r13 + rcx]
    movzx rdx, byte [r13 + rbx]
    add  al, dl
    movzx rax, byte [r13 + rax]
    xor  [r12 + r14], al
    inc  r14
    cmp  r14, r15
    jb   .prga

    add  rsp, 256                 ; liberer S

    ; --- 7. restauration ---
    pop  rax
    pop  rbx
    pop  rcx
    pop  rdx
    pop  rbp
    pop  rsi
    pop  rdi
    pop  r8
    pop  r9
    pop  r10
    pop  r11
    pop  r12
    pop  r13
    pop  r14
    pop  r15

    ; --- 8. saut vers l'entry point original ---
    lea  r11, [rel _start]
    add  r11, [rel entry_delta]
    jmp  r11

; ============ DONNEES (placeholders patches par le packer) ============
key:         times 16 db 0x44            ; placeholder cle (16 octets de 0x44)
code_delta:  dq 0x1111111111111111       ; placeholder distance stub->code
code_len:    dq 0x2222222222222222       ; placeholder longueur
entry_delta: dq 0x3333333333333333       ; placeholder distance stub->entry
msg:         db "....WOODY....", 0x0a
msg_len:     equ $ - msg