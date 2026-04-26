#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

/* ─────────────────────────────────────────────
   Symbol table helpers
   ───────────────────────────────────────────── */

static Symbol *lookupSymbol(SemanticAnalyser *sa, const char *name) {
    Symbol *s = sa->table;
    while (s) {
        if (strcmp(s->name, name) == 0) return s;
        s = s->next;
    }
    return NULL;
}

static void insertSymbol(SemanticAnalyser *sa,
                         const char *name, SymbolType t, int line) {
    Symbol *s = calloc(1, sizeof(Symbol));
    strncpy(s->name, name, sizeof(s->name) - 1);
    s->symType     = t;
    s->declaredLine = line;
    s->next        = sa->table;
    sa->table      = s;
}

/* ─────────────────────────────────────────────
   Type inference from AST node
   Returns SYM_FLOAT if any operand is float, else SYM_INT.
   ───────────────────────────────────────────── */

static SymbolType inferType(SemanticAnalyser *sa, ASTNode *node) {
    if (!node) return SYM_INT;

    switch (node->type) {
        case NODE_FLOAT_LITERAL: return SYM_FLOAT;
        case NODE_INT_LITERAL:   return SYM_INT;

        case NODE_IDENTIFIER: {
            Symbol *s = lookupSymbol(sa, node->value);
            return s ? s->symType : SYM_INT;
        }

        case NODE_BINOP: {
            SymbolType l = inferType(sa, node->left);
            SymbolType r = inferType(sa, node->right);
            return (l == SYM_FLOAT || r == SYM_FLOAT) ? SYM_FLOAT : SYM_INT;
        }

        case NODE_UNOP:
            return inferType(sa, node->left);

        default:
            return SYM_INT;
    }
}

/* ─────────────────────────────────────────────
   AST walker
   ───────────────────────────────────────────── */

static void analyseExpr(SemanticAnalyser *sa, ASTNode *node);
static void analyseStmt(SemanticAnalyser *sa, ASTNode *node);

static void analyseExpr(SemanticAnalyser *sa, ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case NODE_IDENTIFIER: {
            Symbol *s = lookupSymbol(sa, node->value);
            if (!s) {
                // fprintf(stderr,
                //     "[Semantic Error] Line %d: Undeclared variable '%s'\n",
                //     node->line, node->value);
                // sa->errorCount++;
                printf("ERROR: Undeclared variable '%s' at line %d\n",
                    node->value, node->line);
                printf("SOLUTION: Declare the variable before using it.\n\n");
                sa->errorCount++;
            } else {
                s->used = 1;
            }
            break;
        }

        case NODE_BINOP:
            analyseExpr(sa, node->left);
            analyseExpr(sa, node->right);

            /* Division by integer zero */
            if (strcmp(node->value, "/") == 0 &&
                node->right &&
                node->right->type == NODE_INT_LITERAL &&
                strcmp(node->right->value, "0") == 0) {
                fprintf(stderr,
                    "[Semantic Warning] Line %d: Division by zero\n",
                    node->line);
                sa->warningCount++;
            }
            break;

        case NODE_UNOP:
            analyseExpr(sa, node->left);
            break;

        default:
            break;
    }
}

static void analyseBlock(SemanticAnalyser *sa, ASTNode *block);

static void analyseStmt(SemanticAnalyser *sa, ASTNode *node) {
    if (!node) return;

    switch (node->type) {

        // case NODE_VAR_DECL: {
        //     /* Check for redeclaration */
        //     Symbol *existing = lookupSymbol(sa, node->value);
        //     if (existing) {
        //         fprintf(stderr,
        //             "[Semantic Error] Line %d: Redeclaration of variable '%s'"
        //             " (first declared at line %d)\n",
        //             node->line, node->value, existing->declaredLine);
        //         sa->errorCount++;
        //     } else {
        //         /* Determine type from initialiser or default to int */
        //         SymbolType symT = SYM_INT;
        //         if (node->left) {
        //             symT = inferType(sa, node->left);
        //             analyseExpr(sa, node->left);
        //         }
        //         insertSymbol(sa, node->value, symT, node->line);
        //         printf("  [Sym] Declared %-12s  type: %s  line: %d\n",
        //                node->value,
        //                symT == SYM_FLOAT ? "float" : "int",
        //                node->line);
        //     }
        //     break;
        // }
        case NODE_VAR_DECL: {
            Symbol *existing = lookupSymbol(sa, node->value);

            if (existing) {
                printf("[Semantic Error] Line %d: Redeclaration of '%s'\n",
                    node->line, node->value);
                sa->errorCount++;
            } else {
                SymbolType symT = SYM_INT;

                if (node->left) {
                    SymbolType rhs = inferType(sa, node->left);

                    // 🔥 ADD THIS CHECK
                    if (rhs == SYM_FLOAT) {
                        printf("[Semantic Warning] Line %d: Assigning float to int '%s'\n",
                            node->line, node->value);
                        sa->warningCount++;
                    }

                    symT = rhs;
                    analyseExpr(sa, node->left);
                }

                insertSymbol(sa, node->value, symT, node->line);
            }

            break;
        }

        // case NODE_ASSIGN: {
        //     Symbol *s = lookupSymbol(sa, node->left->value);
        //     if (!s) {
        //         fprintf(stderr,
        //             "[Semantic Error] Line %d: Assignment to undeclared"
        //             " variable '%s'\n",
        //             node->line, node->left->value);
        //         sa->errorCount++;
        //     } else {
        //         s->used = 1;
        //         SymbolType rType = inferType(sa, node->right);
        //         /* Implicit int → float is fine; warn on float → int */
        //         if (s->symType == SYM_INT && rType == SYM_FLOAT) {
        //             fprintf(stderr,
        //                 "[Semantic Warning] Line %d: Implicit float-to-int"
        //                 " conversion for '%s'\n",
        //                 node->line, node->left->value);
        //             sa->warningCount++;
        //         }
        //     }
        //     analyseExpr(sa, node->right);
        //     break;
        // }

        case NODE_ASSIGN: {
        // 1. Check RHS expression first
            analyseExpr(sa, node->right);

            // 2. Get LHS symbol (variable)
            Symbol *s = lookupSymbol(sa, node->left->value);

            if (!s) {
                printf("[Semantic Error] Line %d: Undeclared variable '%s'\n",
                    node->line, node->left->value);
                sa->errorCount++;
                return;
            }

            // 3. Get types
            SymbolType lhs = s->symType;
            SymbolType rhs = inferType(sa, node->right);

            // 4. 🔥 ADD THIS CHECK HERE
            if (lhs != rhs) {
                printf("[Semantic Warning] Line %d: Type mismatch (%s <- %s)\n",
                    node->line,
                    lhs == SYM_INT ? "int" : "float",
                    rhs == SYM_INT ? "int" : "float");

                sa->warningCount++;
            }

            break;
        }

        case NODE_IF:
            analyseExpr(sa, node->condition);
            analyseBlock(sa, node->thenBranch);
            if (node->elseBranch) {
                if (node->elseBranch->type == NODE_IF)
                    analyseStmt(sa, node->elseBranch);
                else
                    analyseBlock(sa, node->elseBranch);
            }
            break;

        case NODE_WHILE:
            analyseExpr(sa, node->condition);
            analyseBlock(sa, node->body);
            break;

        case NODE_RETURN:
            analyseExpr(sa, node->left);
            break;

        case NODE_PRINT:
            analyseExpr(sa, node->left);
            break;

        default:
            analyseExpr(sa, node);
            break;
    }
}

static void analyseBlock(SemanticAnalyser *sa, ASTNode *block) {
    if (!block) return;
    ASTNode *stmt = block->body;
    while (stmt) {
        analyseStmt(sa, stmt);
        stmt = stmt->next;
    }
}

/* ─────────────────────────────────────────────
   Unused-variable warnings (after full walk)
   ───────────────────────────────────────────── */

static void checkUnused(SemanticAnalyser *sa) {
    Symbol *s = sa->table;
    while (s) {
        if (!s->used) {
            fprintf(stderr,
                "[Semantic Warning] Variable '%s' declared at line %d"
                " is never used\n",
                s->name, s->declaredLine);
            sa->warningCount++;
        }
        s = s->next;
    }
}

/* ─────────────────────────────────────────────
   Public API
   ───────────────────────────────────────────── */

SemanticAnalyser *createSemanticAnalyser(void) {
    return calloc(1, sizeof(SemanticAnalyser));
}

void analyseProgram(SemanticAnalyser *sa, ASTNode *root) {
    if (!root) return;
    /* root → NODE_PROGRAM → body → NODE_FUNCTION → body → NODE_BLOCK */
    ASTNode *fn = root->body;
    if (fn) analyseBlock(sa, fn->body);
    checkUnused(sa);
}

void freeSemanticAnalyser(SemanticAnalyser *sa) {
    Symbol *s = sa->table;
    while (s) { Symbol *nx = s->next; free(s); s = nx; }
    free(sa);
}

void printSymbolTable(SemanticAnalyser *sa) {
    printf("\n===== SYMBOL TABLE =====\n");
    printf("Name\tType\tLine\tUsed\n");

    Symbol *s = sa->table;
    while (s) {
        printf("%s\t%s\t%d\t%s\n",
               s->name,
               s->symType == SYM_INT ? "int" : "float",
               s->declaredLine,
               s->used ? "yes" : "no");
        s = s->next;
    }
}