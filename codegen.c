#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"



// static char *newTemp(CodeGen *cg) {
//     static char buf[32];
//     snprintf(buf, sizeof(buf), "t%d", ++cg->tempCount);
//     return buf;
// }

// static char *newLabel(CodeGen *cg) {
//     static char buf[32];
//     snprintf(buf, sizeof(buf), "L%d", ++cg->labelCount);
//     return buf;
// }
static char *newTemp(CodeGen *cg) {
    char *buf = (char *)malloc(32);
    sprintf(buf, "t%d", ++cg->tempCount);
    return buf;
}

static char *newLabel(CodeGen *cg) {
    char *buf = (char *)malloc(32);
    sprintf(buf, "L%d", ++cg->labelCount);
    return buf;
}

static TACInstr *newInstr(TACType type) {
    TACInstr *i = calloc(1, sizeof(TACInstr));
    i->type = type;
    return i;
}

static void emit(CodeGen *cg, TACInstr *instr) {
    if (!cg->head) { cg->head = cg->tail = instr; }
    else           { cg->tail->next = instr; cg->tail = instr; }
}


static char *genExpr(CodeGen *cg, ASTNode *node, char *out);
static void  genStmt(CodeGen *cg, ASTNode *node);
static void  genBlock(CodeGen *cg, ASTNode *block);



static char *genExpr(CodeGen *cg, ASTNode *node, char *out) {
    if (!node) { if (out) strcpy(out, "0"); return out; }

    switch (node->type) {

        case NODE_INT_LITERAL:
        case NODE_FLOAT_LITERAL:
            if (out) { strcpy(out, node->value); return out; }
            return node->value;          /* borrow static string */

        case NODE_IDENTIFIER:
            if (out) { strcpy(out, node->value); return out; }
            return node->value;

        case NODE_UNOP: {
            char argBuf[64];
            genExpr(cg, node->left, argBuf);
            char *tmp = newTemp(cg);
            TACInstr *i = newInstr(TAC_UNOP);
            strcpy(i->result, tmp);
            strcpy(i->arg1,   argBuf);
            strcpy(i->op,     node->value);
            emit(cg, i);
            if (out) { strcpy(out, tmp); return out; }
            return tmp;
        }

        case NODE_BINOP: {
            char lBuf[64], rBuf[64];
            genExpr(cg, node->left,  lBuf);
            genExpr(cg, node->right, rBuf);
            char *tmp = newTemp(cg);
            TACInstr *i = newInstr(TAC_BINOP);
            strcpy(i->result, tmp);
            strcpy(i->arg1,   lBuf);
            strcpy(i->arg2,   rBuf);
            strcpy(i->op,     node->value);
            emit(cg, i);
            if (out) { strcpy(out, tmp); return out; }
            return tmp;
        }

        default:
            if (out) { strcpy(out, "?"); return out; }
            return "?";
    }
}



static void genStmt(CodeGen *cg, ASTNode *node) {
    if (!node) return;

    switch (node->type) {

        case NODE_VAR_DECL: {
            if (node->left) {
                char rhs[64];
                genExpr(cg, node->left, rhs);
                TACInstr *i = newInstr(TAC_ASSIGN);
                strcpy(i->result, node->value);
                strcpy(i->arg1,   rhs);
                emit(cg, i);
            }
            break;
        }

        
        case NODE_ASSIGN: {
            char rhs[64];
            genExpr(cg, node->right, rhs);
            TACInstr *i = newInstr(TAC_ASSIGN);
            strcpy(i->result, node->left->value);
            strcpy(i->arg1,   rhs);
            emit(cg, i);
            break;
        }

        /* ----- if / if-else ----- */
        // case NODE_IF: {
        //     char condBuf[64];
        //     genExpr(cg, node->condition, condBuf);

        //     char *lFalse = newLabel(cg);
        //     char *lEnd   = newLabel(cg);

        //     /* if_false cond goto lFalse */
        //     TACInstr *jmp = newInstr(TAC_IF_FALSE);
        //     strcpy(jmp->arg1,   condBuf);
        //     strcpy(jmp->result, lFalse);
        //     emit(cg, jmp);

        //     genBlock(cg, node->thenBranch);

        //     if (node->elseBranch) {
        //         /* goto lEnd */
        //         TACInstr *g = newInstr(TAC_GOTO);
        //         strcpy(g->result, lEnd);
        //         emit(cg, g);

        //         /* lFalse: */
        //         TACInstr *lf = newInstr(TAC_LABEL);
        //         strcpy(lf->result, lFalse);
        //         emit(cg, lf);

        //         if (node->elseBranch->type == NODE_IF)
        //             genStmt(cg, node->elseBranch);
        //         else
        //             genBlock(cg, node->elseBranch);

        //         /* lEnd: */
        //         TACInstr *le = newInstr(TAC_LABEL);
        //         strcpy(le->result, lEnd);
        //         emit(cg, le);
        //     } else {
        //         /* lFalse == lEnd */
        //         TACInstr *lf = newInstr(TAC_LABEL);
        //         strcpy(lf->result, lFalse);
        //         emit(cg, lf);
        //         (void)lEnd;
        //     }
        //     break;
        // }

        case NODE_IF: {
    char condBuf[64];
    genExpr(cg, node->condition, condBuf);

    
    char lFalse[32], lEnd[32];
    snprintf(lFalse, sizeof(lFalse), "L%d", ++cg->labelCount);
    snprintf(lEnd,   sizeof(lEnd),   "L%d", ++cg->labelCount);

    /* if_false cond goto lFalse */
    TACInstr *jmp = newInstr(TAC_IF_FALSE);
    strcpy(jmp->arg1,   condBuf);
    strcpy(jmp->result, lFalse);
    emit(cg, jmp);

    genBlock(cg, node->thenBranch);

    if (node->elseBranch) {
        /* goto lEnd */
        TACInstr *g = newInstr(TAC_GOTO);
        strcpy(g->result, lEnd);
        emit(cg, g);

        /* lFalse: */
        TACInstr *lf = newInstr(TAC_LABEL);
        strcpy(lf->result, lFalse);
        emit(cg, lf);

        if (node->elseBranch->type == NODE_IF)
            genStmt(cg, node->elseBranch);
        else
            genBlock(cg, node->elseBranch);

        /* lEnd: */
        TACInstr *le = newInstr(TAC_LABEL);
        strcpy(le->result, lEnd);
        emit(cg, le);
    } else {
        /* no else — lFalse is the end */
        TACInstr *lf = newInstr(TAC_LABEL);
        strcpy(lf->result, lFalse);
        emit(cg, lf);
    }
    break;
}

        /* ----- while ----- */
        // case NODE_WHILE: {
        //     char *lStart = newLabel(cg);
        //     char *lEnd   = newLabel(cg);
        case NODE_WHILE: {
            char lStart[32], lEnd[32];
            snprintf(lStart, sizeof(lStart), "L%d", ++cg->labelCount);
            snprintf(lEnd,   sizeof(lEnd),   "L%d", ++cg->labelCount);

            /* lStart: */
            TACInstr *ls = newInstr(TAC_LABEL);
            strcpy(ls->result, lStart);
            emit(cg, ls);

            char condBuf[64];
            genExpr(cg, node->condition, condBuf);

            /* if_false cond goto lEnd */
            TACInstr *jmp = newInstr(TAC_IF_FALSE);
            strcpy(jmp->arg1,   condBuf);
            strcpy(jmp->result, lEnd);
            emit(cg, jmp);

            genBlock(cg, node->body);

            /* goto lStart */
            TACInstr *g = newInstr(TAC_GOTO);
            strcpy(g->result, lStart);
            emit(cg, g);

            /* lEnd: */
            TACInstr *le = newInstr(TAC_LABEL);
            strcpy(le->result, lEnd);
            emit(cg, le);
            break;
        }

        /* ----- return ----- */
        case NODE_RETURN: {
            char val[64] = "0";
            if (node->left) genExpr(cg, node->left, val);
            TACInstr *i = newInstr(TAC_RETURN);
            strcpy(i->arg1, val);
            emit(cg, i);
            break;
        }

        /* ----- print ----- */
        case NODE_PRINT: {
            char val[64];
            genExpr(cg, node->left, val);
            TACInstr *i = newInstr(TAC_PRINT);
            strcpy(i->arg1, val);
            emit(cg, i);
            break;
        }

        default:
            break;
    }
}

static void genBlock(CodeGen *cg, ASTNode *block) {
    if (!block) return;
    ASTNode *s = block->body;
    while (s) { genStmt(cg, s); s = s->next; }
}

/* ─────────────────────────────────────────────
   Public API
   ───────────────────────────────────────────── */

CodeGen *createCodeGen(void) {
    return calloc(1, sizeof(CodeGen));
}

void generateCode(CodeGen *cg, ASTNode *root) {
    if (!root) return;
    ASTNode *fn = root->body;    /* NODE_FUNCTION */
    if (!fn) return;

    TACInstr *begin = newInstr(TAC_FUNC_BEGIN);
    strcpy(begin->result, fn->value);
    emit(cg, begin);

    genBlock(cg, fn->body);

    TACInstr *end = newInstr(TAC_FUNC_END);
    strcpy(end->result, fn->value);
    emit(cg, end);
}

void printTAC(CodeGen *cg) {
    TACInstr *i = cg->head;
    int instrNo  = 1;
    while (i) {
        switch (i->type) {
            case TAC_FUNC_BEGIN:
                printf("  BEGIN %s\n", i->result);
                break;
            case TAC_FUNC_END:
                printf("  END   %s\n", i->result);
                break;
            case TAC_LABEL:
                printf("%s:\n", i->result);
                instrNo--;   /* labels don't get a number */
                break;
            case TAC_ASSIGN:
                printf("  (%3d)  %s = %s\n", instrNo, i->result, i->arg1);
                break;
            case TAC_BINOP:
                printf("  (%3d)  %s = %s %s %s\n",
                       instrNo, i->result, i->arg1, i->op, i->arg2);
                break;
            case TAC_UNOP:
                printf("  (%3d)  %s = %s%s\n",
                       instrNo, i->result, i->op, i->arg1);
                break;
            case TAC_GOTO:
                printf("  (%3d)  goto %s\n", instrNo, i->result);
                break;
            case TAC_IF_FALSE:
                printf("  (%3d)  if_false %s goto %s\n",
                       instrNo, i->arg1, i->result);
                break;
            case TAC_PRINT:
                printf("  (%3d)  print %s\n", instrNo, i->arg1);
                break;
            case TAC_RETURN:
                printf("  (%3d)  return %s\n", instrNo, i->arg1);
                break;
            default:
                break;
        }
        instrNo++;
        i = i->next;
    }
}

void freeCodeGen(CodeGen *cg) {
    TACInstr *i = cg->head;
    while (i) { TACInstr *nx = i->next; free(i); i = nx; }
    free(cg);
}
