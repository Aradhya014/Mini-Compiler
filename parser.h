#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <stdio.h>

/* ── AST Node Types ── */
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_BLOCK,
    NODE_VAR_DECL,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_RETURN,
    NODE_PRINT,
    NODE_BINOP,
    NODE_UNOP,
    NODE_INT_LITERAL,
    NODE_FLOAT_LITERAL,
    NODE_IDENTIFIER,
    NODE_CALL
} NodeType;

/* ── AST Node ── */
typedef struct ASTNode {
    NodeType type;
    char     value[100];   

    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *condition;
    struct ASTNode *thenBranch;
    struct ASTNode *elseBranch;
    struct ASTNode *body;
    struct ASTNode *next;    

    int line;
} ASTNode;


typedef struct {
    FILE  *fp;
    Token  current;
    int    errorCount;
} Parser;

/* ── Public API ── */
Parser  *createParser(FILE *fp);
ASTNode *parse(Parser *p);
void     printAST(ASTNode *node, int depth);
void     freeAST(ASTNode *node);
void writeDOT(ASTNode *root);
void generateDOT(ASTNode *root, FILE *fp);

#endif /* PARSER_H */