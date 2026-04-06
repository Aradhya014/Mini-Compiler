#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"

/* ── Symbol kinds ── */
typedef enum { SYM_INT, SYM_FLOAT } SymbolType;

/* ── Symbol table entry ── */
typedef struct Symbol {
    char        name[100];
    SymbolType  symType;
    int         declaredLine;
    int         used;
    struct Symbol *next;
} Symbol;

/* ── Semantic analyser state ── */
typedef struct {
    Symbol *table;
    int     errorCount;
    int     warningCount;
} SemanticAnalyser;

/* ── Public API ── */
SemanticAnalyser *createSemanticAnalyser(void);
void              analyseProgram(SemanticAnalyser *sa, ASTNode *root);
void              freeSemanticAnalyser(SemanticAnalyser *sa);

#endif /* SEMANTIC_H */