#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"

/* ── TAC instruction types ── */
typedef enum {
    TAC_ASSIGN,    /* x = y            */
    TAC_BINOP,     /* x = y op z       */
    TAC_UNOP,      /* x = -y           */
    TAC_COPY,      /* x = y            */
    TAC_LABEL,     /* L1:              */
    TAC_GOTO,      /* goto L           */
    TAC_IF_FALSE,  /* if_false x goto L*/
    TAC_PRINT,     /* print x          */
    TAC_RETURN,    /* return x         */
    TAC_FUNC_BEGIN,/* begin func       */
    TAC_FUNC_END   /* end func         */
} TACType;

/* ── Single TAC instruction ── */
typedef struct TACInstr {
    TACType type;
    char    result[64];
    char    arg1[64];
    char    arg2[64];
    char    op[8];
    struct TACInstr *next;
} TACInstr;

/* ── Code generator state ── */
typedef struct {
    TACInstr *head;
    TACInstr *tail;
    int       tempCount;
    int       labelCount;
} CodeGen;

/* ── Public API ── */
CodeGen *createCodeGen(void);
void     generateCode(CodeGen *cg, ASTNode *root);
void     printTAC(CodeGen *cg);
void     freeCodeGen(CodeGen *cg);

#endif /* CODEGEN_H */
