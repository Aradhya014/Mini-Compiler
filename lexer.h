// #ifndef LEXER_H
// #define LEXER_H

// #include <stdio.h>

// typedef enum {
//     TOKEN_INT,
//     TOKEN_FLOAT,        
//     TOKEN_IF,
//     TOKEN_ELSE,
//     TOKEN_WHILE,
//     TOKEN_RETURN,
//     TOKEN_PRINT,
//     TOKEN_MAIN,

//     TOKEN_IDENTIFIER,
//     TOKEN_INT_LITERAL,      
//     TOKEN_FLOAT_LITERAL,    

//     TOKEN_PLUS,
//     TOKEN_MINUS,
//     TOKEN_MUL,
//     TOKEN_DIV,

//     TOKEN_ASSIGN,
//     TOKEN_EQ,
//     TOKEN_NEQ,
//     TOKEN_LT,
//     TOKEN_GT,
//     TOKEN_LE,
//     TOKEN_GE,

//     TOKEN_LPAREN,
//     TOKEN_RPAREN,
//     TOKEN_LBRACE,
//     TOKEN_RBRACE,
//     TOKEN_SEMICOLON,
//     TOKEN_COMMA,

//     TOKEN_EOF,
//     TOKEN_UNKNOWN
// } TokenType;

// typedef struct {
//     TokenType type;
//     char lexeme[100];
//     int line;
// } Token;

// Token getNextToken(FILE *fp);
// const char* tokenTypeToString(TokenType type);

// #endif

#ifndef LEXER_H
#define LEXER_H
 
#include <stdio.h>
 
typedef enum {
    TOKEN_INT,
    TOKEN_FLOAT,        
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_RETURN,
    TOKEN_PRINT,
    TOKEN_MAIN,
 
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,      
    TOKEN_FLOAT_LITERAL,    
 
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MUL,
    TOKEN_DIV,
 
    TOKEN_ASSIGN,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE,
 
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
 
    TOKEN_EOF,
    TOKEN_UNKNOWN,
    TOKEN_INVALID   
} TokenType;
 
typedef struct {
    TokenType type;
    char lexeme[100];
    int line;
} Token;
 
Token getNextToken(FILE *fp);
const char* tokenTypeToString(TokenType type);
 

extern int lexErrorCount;
 
#endif
 