#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "optimizer.h"

/* ─────────────────────────────────────────────
   Utility: is a string a numeric literal?
   ───────────────────────────────────────────── */
static int isNumeric(const char *s) {
    if (!s || !*s) return 0;
    const char *p = s;
    if (*p == '-') p++;
    int hasDot = 0, digits = 0;
    while (*p) {
        if (*p == '.') { if (hasDot++) return 0; }
        else if (isdigit((unsigned char)*p)) digits++;
        else return 0;
        p++;
    }
    return digits > 0;
}

static double toDouble(const char *s) { return atof(s); }

static void formatNumber(char *buf, double v) {
    /* Use integer form if it's a whole number */
    if (v == (long long)v)
        snprintf(buf, 64, "%lld", (long long)v);
    else
        snprintf(buf, 64, "%g", v);
}

/* ─────────────────────────────────────────────
   Pass 1: Constant Folding
   Replace BINOP/UNOP on literals with a single ASSIGN literal
   ───────────────────────────────────────────── */
static int constantFolding(CodeGen *cg) {
    int count = 0;
    TACInstr *i = cg->head;
    while (i) {
        if (i->type == TAC_BINOP &&
            isNumeric(i->arg1) && isNumeric(i->arg2)) {

            double a = toDouble(i->arg1);
            double b = toDouble(i->arg2);
            double r = 0.0;
            int    ok = 1;

            if      (strcmp(i->op, "+")  == 0) r = a + b;
            else if (strcmp(i->op, "-")  == 0) r = a - b;
            else if (strcmp(i->op, "*")  == 0) r = a * b;
            else if (strcmp(i->op, "/")  == 0) {
                if (b == 0.0) { ok = 0; }
                else r = a / b;
            }
            /* Relational: result is 0 or 1 */
            else if (strcmp(i->op, "==") == 0) r = (a == b);
            else if (strcmp(i->op, "!=") == 0) r = (a != b);
            else if (strcmp(i->op, "<")  == 0) r = (a <  b);
            else if (strcmp(i->op, ">")  == 0) r = (a >  b);
            else if (strcmp(i->op, "<=") == 0) r = (a <= b);
            else if (strcmp(i->op, ">=") == 0) r = (a >= b);
            else ok = 0;

            if (ok) {
                char buf[64];
                formatNumber(buf, r);
                i->type = TAC_ASSIGN;
                strcpy(i->arg1, buf);
                i->arg2[0] = '\0';
                i->op[0]   = '\0';
                count++;
                printf("  [Opt] Folded: %s = %s\n", i->result, buf);
            }
        }

        if (i->type == TAC_UNOP && strcmp(i->op, "-") == 0 &&
            isNumeric(i->arg1)) {
            double a = -toDouble(i->arg1);
            char buf[64];
            formatNumber(buf, a);
            i->type = TAC_ASSIGN;
            strcpy(i->arg1, buf);
            i->op[0] = '\0';
            count++;
            printf("  [Opt] Folded unary: %s = %s\n", i->result, buf);
        }

        i = i->next;
    }
    return count;
}

/* ─────────────────────────────────────────────
   Pass 2: Dead Assignment Elimination
   A temp tN is "dead" if it is never used as arg1/arg2 by any later instr.
   We only remove temps (tN), not user variables.
   ───────────────────────────────────────────── */
static int isTemp(const char *s) {
    return s && s[0] == 't' && isdigit((unsigned char)s[1]);
}

static int isUsed(CodeGen *cg, const char *name, TACInstr *from) {
    TACInstr *i = from;
    while (i) {
        if (strcmp(i->arg1,   name) == 0) return 1;
        if (strcmp(i->arg2,   name) == 0) return 1;
        if (strcmp(i->result, name) == 0 &&
            (i->type == TAC_IF_FALSE || i->type == TAC_GOTO)) return 1;
        i = i->next;
    }
    return 0;
}

static int deadCodeElimination(CodeGen *cg) {
    int count = 0;
    TACInstr *prev = NULL;
    TACInstr *i    = cg->head;

    while (i) {
        int kill = 0;
        if ((i->type == TAC_ASSIGN || i->type == TAC_BINOP ||
             i->type == TAC_UNOP)  && isTemp(i->result)) {
            if (!isUsed(cg, i->result, i->next)) {
                kill = 1;
            }
        }

        if (kill) {
            printf("  [Opt] Dead assignment removed: %s\n", i->result);
            count++;
            TACInstr *dead = i;
            if (prev) prev->next = i->next;
            else       cg->head  = i->next;
            if (cg->tail == dead) cg->tail = prev;
            i = i->next;
            free(dead);
        } else {
            prev = i;
            i    = i->next;
        }
    }
    return count;
}

/* ─────────────────────────────────────────────
   Public API
   ───────────────────────────────────────────── */

OptStats optimizeTAC(CodeGen *cg) {
    OptStats stats = {0, 0};
    stats.foldedConstants     = constantFolding(cg);
    stats.eliminatedDeadCode  = deadCodeElimination(cg);
    return stats;
}
