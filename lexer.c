#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"

int currentLine = 1;

const char* tokenTypeToString(TokenType type) {
    switch(type) {
        case TOKEN_INT: return "TOKEN_INT";
        case TOKEN_FLOAT: return "TOKEN_FLOAT";
        case TOKEN_IF: return "TOKEN_IF";
        case TOKEN_ELSE: return "TOKEN_ELSE";
        case TOKEN_WHILE: return "TOKEN_WHILE";
        case TOKEN_RETURN: return "TOKEN_RETURN";
        case TOKEN_PRINT: return "TOKEN_PRINT";
        case TOKEN_MAIN: return "TOKEN_MAIN";
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_INT_LITERAL: return "TOKEN_INT_LITERAL";
        case TOKEN_FLOAT_LITERAL: return "TOKEN_FLOAT_LITERAL";
        case TOKEN_PLUS: return "TOKEN_PLUS";
        case TOKEN_MINUS: return "TOKEN_MINUS";
        case TOKEN_MUL: return "TOKEN_MUL";
        case TOKEN_DIV: return "TOKEN_DIV";
        case TOKEN_ASSIGN: return "TOKEN_ASSIGN";
        case TOKEN_EQ: return "TOKEN_EQ";
        case TOKEN_NEQ: return "TOKEN_NEQ";
        case TOKEN_LT: return "TOKEN_LT";
        case TOKEN_GT: return "TOKEN_GT";
        case TOKEN_LE: return "TOKEN_LE";
        case TOKEN_GE: return "TOKEN_GE";
        case TOKEN_LPAREN: return "TOKEN_LPAREN";
        case TOKEN_RPAREN: return "TOKEN_RPAREN";
        case TOKEN_LBRACE: return "TOKEN_LBRACE";
        case TOKEN_RBRACE: return "TOKEN_RBRACE";
        case TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";
        case TOKEN_COMMA: return "TOKEN_COMMA";
        case TOKEN_EOF: return "TOKEN_EOF";
        default: return "TOKEN_UNKNOWN";
    }
}

Token getNextToken(FILE *fp) {
    Token token;
    int c;

    while ((c = fgetc(fp)) != EOF) {

        if (c == '\n') {
            currentLine++;
            continue;
        }

        if (isspace(c))
            continue;

        token.line = currentLine;

        
        if (isalpha(c)) {
            int i = 0;
            token.lexeme[i++] = c;

            while (isalnum(c = fgetc(fp))) {
                token.lexeme[i++] = c;
            }

            token.lexeme[i] = '\0';
            ungetc(c, fp);

            if (strcmp(token.lexeme, "int") == 0)
                token.type = TOKEN_INT;
            else if (strcmp(token.lexeme, "float") == 0)
                token.type = TOKEN_FLOAT;   // NEW
            else if (strcmp(token.lexeme, "if") == 0)
                token.type = TOKEN_IF;
            else if (strcmp(token.lexeme, "else") == 0)
                token.type = TOKEN_ELSE;
            else if (strcmp(token.lexeme, "while") == 0)
                token.type = TOKEN_WHILE;
            else if (strcmp(token.lexeme, "return") == 0)
                token.type = TOKEN_RETURN;
            else if (strcmp(token.lexeme, "print") == 0)
                token.type = TOKEN_PRINT;
            else if (strcmp(token.lexeme, "main") == 0)
                token.type = TOKEN_MAIN;
            else
                token.type = TOKEN_IDENTIFIER;

            return token;
        }

        
        if (isdigit(c)) {
            int i = 0;
            int isFloat = 0;
            token.lexeme[i++] = c;

            while (1) {
                c = fgetc(fp);

                if (isdigit(c)) {
                    token.lexeme[i++] = c;
                }
                else if (c == '.') {
                    if (isFloat) break;  
                    isFloat = 1;
                    token.lexeme[i++] = c;
                }
                else {
                    ungetc(c, fp);
                    break;
                }
            }

            token.lexeme[i] = '\0';

            if (isFloat)
                token.type = TOKEN_FLOAT_LITERAL;
            else
                token.type = TOKEN_INT_LITERAL;

            return token;
        }

        
        if (c == '=') {
            int next = fgetc(fp);
            if (next == '=') {
                token.type = TOKEN_EQ;
                strcpy(token.lexeme, "==");
            } else {
                token.type = TOKEN_ASSIGN;
                token.lexeme[0] = '=';
                token.lexeme[1] = '\0';
                ungetc(next, fp);
            }
            return token;
        }

        if (c == '!') {
            int next = fgetc(fp);
            if (next == '=') {
                token.type = TOKEN_NEQ;
                strcpy(token.lexeme, "!=");
                return token;
            }
        }

        if (c == '<') {
            int next = fgetc(fp);
            if (next == '=') {
                token.type = TOKEN_LE;
                strcpy(token.lexeme, "<=");
            } else {
                token.type = TOKEN_LT;
                token.lexeme[0] = '<';
                token.lexeme[1] = '\0';
                ungetc(next, fp);
            }
            return token;
        }

        if (c == '>') {
            int next = fgetc(fp);
            if (next == '=') {
                token.type = TOKEN_GE;
                strcpy(token.lexeme, ">=");
            } else {
                token.type = TOKEN_GT;
                token.lexeme[0] = '>';
                token.lexeme[1] = '\0';
                ungetc(next, fp);
            }
            return token;
        }

        
        switch (c) {
            case '+': token.type = TOKEN_PLUS; break;
            case '-': token.type = TOKEN_MINUS; break;
            case '*': token.type = TOKEN_MUL; break;
            case '/': token.type = TOKEN_DIV; break;
            case '(': token.type = TOKEN_LPAREN; break;
            case ')': token.type = TOKEN_RPAREN; break;
            case '{': token.type = TOKEN_LBRACE; break;
            case '}': token.type = TOKEN_RBRACE; break;
            case ';': token.type = TOKEN_SEMICOLON; break;
            case ',': token.type = TOKEN_COMMA; break;
            default:
                printf("Lexical Error at line %d: Unknown symbol '%c'\n", currentLine, c);
                token.type = TOKEN_UNKNOWN;
                token.lexeme[0] = c;
                token.lexeme[1] = '\0';
                return token;
        }

        token.lexeme[0] = c;
        token.lexeme[1] = '\0';
        return token;
    }

    token.type = TOKEN_EOF;
    strcpy(token.lexeme, "EOF");
    token.line = currentLine;
    return token;
}
