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
    mov     eax, 10
    mov     ebx, eax
    mov     ecx, 9
    mov     edx, ecx
    mov     esi, ebx
    add     esi, edx
    mov     edi, esi
    mov     r8, 200
    mov     r9, r8
    mov     eax, edi
    add     eax, 10
    mov     ebx, eax
    ; print result
    mov     rdi, fmt_int
    mov     rsi, ebx
    xor     eax, eax
    call    printf
    mov     ecx, 9
L2:
    mov     edx, ecx
    cmp     edx, 3
    setg     al
    movzx   edx, al
    cmp     edx, 0
    je      L2
    ; print b
    mov     rdi, fmt_int
    mov     rsi, ecx
    xor     eax, eax
    call    printf
    jmp     L2
L2:
    mov     eax, 0
    pop     rbp
    ret
