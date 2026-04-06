#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "target.h"

/* ─────────────────────────────────────────────
   Register allocator (trivial: cycle through eax,ebx,ecx,edx)
   Maps variable/temp names → register names.
   ───────────────────────────────────────────── */

#define MAX_VARS 128
static struct { char name[64]; char reg[8]; } regMap[MAX_VARS];
static int regCount = 0;

static const char *regs[] = { "eax", "ebx", "ecx", "edx",
                               "esi", "edi", "r8",  "r9" };
#define NREGS 8

static const char *allocReg(const char *name) {
    for (int i = 0; i < regCount; i++)
        if (strcmp(regMap[i].name, name) == 0) return regMap[i].reg;
    if (regCount < MAX_VARS) {
        strncpy(regMap[regCount].name, name, 63);
        strncpy(regMap[regCount].reg,  regs[regCount % NREGS], 7);
        return regMap[regCount++].reg;
    }
    return "eax";  /* fallback */
}

/* ─────────────────────────────────────────────
   Resolve operand: literal → immediate, else register
   ───────────────────────────────────────────── */
static int isLiteral(const char *s) {
    if (!s || !*s) return 0;
    const char *p = s;
    if (*p == '-') p++;
    int dot = 0, d = 0;
    while (*p) {
        if (*p == '.') { if (dot++) return 0; }
        else if (isdigit((unsigned char)*p)) d++;
        else return 0;
        p++;
    }
    return d > 0;
}

static const char *operand(const char *name) {
    if (isLiteral(name)) return name;  /* immediate */
    return allocReg(name);
}

/* ─────────────────────────────────────────────
   Emit one assembly instruction
   ───────────────────────────────────────────── */
static void asmLine(FILE *fp, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(fp, "    ");
    vfprintf(fp, fmt, ap);
    fprintf(fp, "\n");
    va_end(ap);
}

/* ─────────────────────────────────────────────
   Main generator
   ───────────────────────────────────────────── */
void generateAssembly(CodeGen *cg, const char *outFile) {
    FILE *fp = fopen(outFile, "w");
    if (!fp) { perror("Cannot open output file"); return; }

    fprintf(fp, "; ═══════════════════════════════════════════\n");
    fprintf(fp, "; Generated Assembly  (x86-like pseudo-asm)\n");
    fprintf(fp, "; Mini-C Compiler — Target Code Generation\n");
    fprintf(fp, "; ═══════════════════════════════════════════\n\n");
    fprintf(fp, "section .data\n");
    fprintf(fp, "    fmt_int   db \"%%d\", 10, 0\n");
    fprintf(fp, "    fmt_float db \"%%f\", 10, 0\n\n");
    fprintf(fp, "section .text\n");
    fprintf(fp, "    global main\n\n");

    TACInstr *i = cg->head;
    while (i) {
        switch (i->type) {

            case TAC_FUNC_BEGIN:
                fprintf(fp, "%s:\n", i->result);
                fprintf(fp, "    push    rbp\n");
                fprintf(fp, "    mov     rbp, rsp\n");
                break;

            case TAC_FUNC_END:
                fprintf(fp, "    pop     rbp\n");
                fprintf(fp, "    ret\n");
                break;

            case TAC_LABEL:
                fprintf(fp, "%s:\n", i->result);
                break;

            case TAC_ASSIGN: {
                const char *dst = allocReg(i->result);
                const char *src = operand(i->arg1);
                if (isLiteral(i->arg1))
                    fprintf(fp, "    mov     %s, %s\n", dst, src);
                else
                    fprintf(fp, "    mov     %s, %s\n", dst, src);
                break;
            }

            case TAC_BINOP: {
                const char *dst = allocReg(i->result);
                const char *a   = operand(i->arg1);
                const char *b   = operand(i->arg2);

                /* load first operand into dst */
                fprintf(fp, "    mov     %s, %s\n", dst, a);

                if (strcmp(i->op, "+") == 0)
                    fprintf(fp, "    add     %s, %s\n", dst, b);
                else if (strcmp(i->op, "-") == 0)
                    fprintf(fp, "    sub     %s, %s\n", dst, b);
                else if (strcmp(i->op, "*") == 0)
                    fprintf(fp, "    imul    %s, %s\n", dst, b);
                else if (strcmp(i->op, "/") == 0) {
                    fprintf(fp, "    ; idiv requires rax:rdx\n");
                    fprintf(fp, "    mov     eax, %s\n", dst);
                    fprintf(fp, "    cdq\n");
                    fprintf(fp, "    idiv    %s\n", b);
                    fprintf(fp, "    mov     %s, eax\n", dst);
                }
                /* Relational — use cmp + setX */
                else {
                    fprintf(fp, "    cmp     %s, %s\n", dst, b);
                    const char *setInstr = "sete";
                    if      (strcmp(i->op, "==") == 0) setInstr = "sete";
                    else if (strcmp(i->op, "!=") == 0) setInstr = "setne";
                    else if (strcmp(i->op, "<")  == 0) setInstr = "setl";
                    else if (strcmp(i->op, ">")  == 0) setInstr = "setg";
                    else if (strcmp(i->op, "<=") == 0) setInstr = "setle";
                    else if (strcmp(i->op, ">=") == 0) setInstr = "setge";
                    fprintf(fp, "    %-8s al\n",  setInstr);
                    fprintf(fp, "    movzx   %s, al\n", dst);
                }
                break;
            }

            case TAC_UNOP: {
                const char *dst = allocReg(i->result);
                const char *a   = operand(i->arg1);
                fprintf(fp, "    mov     %s, %s\n", dst, a);
                if (strcmp(i->op, "-") == 0)
                    fprintf(fp, "    neg     %s\n", dst);
                break;
            }

            case TAC_GOTO:
                fprintf(fp, "    jmp     %s\n", i->result);
                break;

            case TAC_IF_FALSE: {
                const char *cond = operand(i->arg1);
                fprintf(fp, "    cmp     %s, 0\n", cond);
                fprintf(fp, "    je      %s\n", i->result);
                break;
            }

            case TAC_PRINT: {
                const char *src = operand(i->arg1);
                fprintf(fp, "    ; print %s\n", i->arg1);
                fprintf(fp, "    mov     rdi, fmt_int\n");
                fprintf(fp, "    mov     rsi, %s\n", src);
                fprintf(fp, "    xor     eax, eax\n");
                fprintf(fp, "    call    printf\n");
                break;
            }

            case TAC_RETURN: {
                const char *src = operand(i->arg1);
                if (isLiteral(i->arg1))
                    fprintf(fp, "    mov     eax, %s\n", src);
                else
                    fprintf(fp, "    mov     eax, %s\n", src);
                break;
            }

            default:
                break;
        }
        i = i->next;
    }

    fclose(fp);
}
