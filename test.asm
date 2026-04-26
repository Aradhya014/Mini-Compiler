; ═══════════════════════════════════════════
; Generated Assembly  (x86-like pseudo-asm)
; Mini-C Compiler — Target Code Generation
; ═══════════════════════════════════════════

section .data
    fmt_int   db "%d", 10, 0
    fmt_float db "%f", 10, 0

section .text
    global main

main:
    push    rbp
    mov     rbp, rsp
    mov     eax, 3
    mov     ebx, 2
    mov     ecx, eax
    add     ecx, ebx
    mov     edx, ecx
    ; print z
    mov     rdi, fmt_int
    mov     rsi, edx
    xor     eax, eax
    call    printf
    mov     eax, 0
    pop     rbp
    ret
