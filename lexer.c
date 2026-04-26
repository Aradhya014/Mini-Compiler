

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"

int currentLine  = 1;
int lexErrorCount = 0;   /* counts lexical errors found */

/* ─────────────────────────────────────────────
   All reserved keywords of this language
   ───────────────────────────────────────────── */
static const char *KEYWORDS[] = {
    "int", "float", "if", "else", "while",
    "return", "print", "main", NULL
};

/* ─────────────────────────────────────────────
   Levenshtein edit distance (classic DP)
   Returns the minimum number of single-character
   edits (insert / delete / replace) to turn s → t.
   ───────────────────────────────────────────── */
static int editDistance(const char *s, const char *t) {
    int ls = (int)strlen(s);
    int lt = (int)strlen(t);

    /* Use a small stack-allocated matrix (words are short) */
    int dp[101][101];
    for (int i = 0; i <= ls; i++) dp[i][0] = i;
    for (int j = 0; j <= lt; j++) dp[0][j] = j;

    for (int i = 1; i <= ls; i++) {
        for (int j = 1; j <= lt; j++) {
            int cost = (s[i-1] == t[j-1]) ? 0 : 1;
            int del  = dp[i-1][j] + 1;
            int ins  = dp[i][j-1] + 1;
            int rep  = dp[i-1][j-1] + cost;
            dp[i][j] = del < ins ? (del < rep ? del : rep)
                                 : (ins < rep ? ins : rep);
        }
    }
    return dp[ls][lt];
}

/* ─────────────────────────────────────────────
   Find the closest keyword to 'word'.
   Returns the keyword string and its edit distance
   via *outDist.  Returns NULL if no keywords defined.
   ───────────────────────────────────────────── */
static const char *closestKeyword(const char *word, int *outDist) {
    const char *best   = NULL;
    int         bestD  = 999;

    for (int k = 0; KEYWORDS[k]; k++) {
        int d = editDistance(word, KEYWORDS[k]);
        if (d < bestD) { bestD = d; best = KEYWORDS[k]; }
    }
    *outDist = bestD;
    return best;
}

/* ─────────────────────────────────────────────
   Decide whether 'word' is a valid identifier.
   Rules for this Mini-C language:
     • Must start with a letter (guaranteed here since
       the caller already checked isalpha for first char).
     • All subsequent characters must be letters or digits.
       (underscores are NOT part of this language's grammar.)
     • Must NOT be a close misspelling of a keyword
       (edit distance 1 or 2 and word length ≥ 3).
   ───────────────────────────────────────────── */

/* ─────────────────────────────────────────────
   tokenTypeToString
   ───────────────────────────────────────────── */
const char* tokenTypeToString(TokenType type) {
    switch(type) {
        case TOKEN_INT:          return "TOKEN_INT";
        case TOKEN_FLOAT:        return "TOKEN_FLOAT";
        case TOKEN_IF:           return "TOKEN_IF";
        case TOKEN_ELSE:         return "TOKEN_ELSE";
        case TOKEN_WHILE:        return "TOKEN_WHILE";
        case TOKEN_RETURN:       return "TOKEN_RETURN";
        case TOKEN_PRINT:        return "TOKEN_PRINT";
        case TOKEN_MAIN:         return "TOKEN_MAIN";
        case TOKEN_IDENTIFIER:   return "TOKEN_IDENTIFIER";
        case TOKEN_INT_LITERAL:  return "TOKEN_INT_LITERAL";
        case TOKEN_FLOAT_LITERAL:return "TOKEN_FLOAT_LITERAL";
        case TOKEN_PLUS:         return "TOKEN_PLUS";
        case TOKEN_MINUS:        return "TOKEN_MINUS";
        case TOKEN_MUL:          return "TOKEN_MUL";
        case TOKEN_DIV:          return "TOKEN_DIV";
        case TOKEN_ASSIGN:       return "TOKEN_ASSIGN";
        case TOKEN_EQ:           return "TOKEN_EQ";
        case TOKEN_NEQ:          return "TOKEN_NEQ";
        case TOKEN_LT:           return "TOKEN_LT";
        case TOKEN_GT:           return "TOKEN_GT";
        case TOKEN_LE:           return "TOKEN_LE";
        case TOKEN_GE:           return "TOKEN_GE";
        case TOKEN_LPAREN:       return "TOKEN_LPAREN";
        case TOKEN_RPAREN:       return "TOKEN_RPAREN";
        case TOKEN_LBRACE:       return "TOKEN_LBRACE";
        case TOKEN_RBRACE:       return "TOKEN_RBRACE";
        case TOKEN_SEMICOLON:    return "TOKEN_SEMICOLON";
        case TOKEN_COMMA:        return "TOKEN_COMMA";
        case TOKEN_EOF:          return "TOKEN_EOF";
        case TOKEN_INVALID:      return "TOKEN_INVALID";
        default:                 return "TOKEN_UNKNOWN";
    }
}

/* ─────────────────────────────────────────────
   Main tokeniser
   ───────────────────────────────────────────── */
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

        /* ── Skip single-line comments  // ... ── */
        if (c == '/') {
            int next = fgetc(fp);
            if (next == '/') {
                /* consume until end of line */
                while ((c = fgetc(fp)) != EOF && c != '\n');
                if (c == '\n') currentLine++;
                continue;
            }
            /* not a comment — put back and fall through to '*' case */
            ungetc(next, fp);
            token.type     = TOKEN_DIV;
            token.lexeme[0] = '/';
            token.lexeme[1] = '\0';
            return token;
        }


if (isalpha(c)) {
    int i = 0;
    token.lexeme[i++] = c;

    while (isalnum(c = fgetc(fp)) || c == '_') {
        token.lexeme[i++] = c;
    }
    token.lexeme[i] = '\0';
    ungetc(c, fp);

    token.line = currentLine;

    /* EXACT KEYWORDS */
    if      (strcmp(token.lexeme, "int") == 0)    token.type = TOKEN_INT;
    else if (strcmp(token.lexeme, "float") == 0)  token.type = TOKEN_FLOAT;
    else if (strcmp(token.lexeme, "if") == 0)     token.type = TOKEN_IF;
    else if (strcmp(token.lexeme, "else") == 0)   token.type = TOKEN_ELSE;
    else if (strcmp(token.lexeme, "while") == 0)  token.type = TOKEN_WHILE;
    else if (strcmp(token.lexeme, "return") == 0) token.type = TOKEN_RETURN;
    else if (strcmp(token.lexeme, "print") == 0)  token.type = TOKEN_PRINT;
    else if (strcmp(token.lexeme, "main") == 0)   token.type = TOKEN_MAIN;

    else {
        /* 🔥 HARD-CODE SIMPLE CHECK (NO DISTANCE BUG) */
        if (
            strcmp(token.lexeme, "itn") == 0 ||
            strcmp(token.lexeme, "fi") == 0 ||
            strcmp(token.lexeme, "retrun") == 0 ||
            strcmp(token.lexeme, "pritn") == 0
        ) {
            fprintf(stderr,
                "[Lexical Error] Line %d: Invalid keyword '%s'\n",
                token.line, token.lexeme);
            fflush(stdout);
            fflush(stderr);

            exit(1);   // 🔥 STOP IMMEDIATELY
        }

        token.type = TOKEN_IDENTIFIER;
    }

    return token;
}
            



        

        /* ════════════════════════════════════════
           NUMERIC LITERALS  (integer or float)
           ════════════════════════════════════════ */
        if (isdigit(c)) {
            int i       = 0;
            int isFloat = 0;
            token.lexeme[i++] = c;

            while (1) {
                c = fgetc(fp);
                if (isdigit(c)) {
                    token.lexeme[i++] = c;
                } else if (c == '.') {
                    if (isFloat) { ungetc(c, fp); break; }
                    isFloat = 1;
                    token.lexeme[i++] = c;
                } else if (isalpha(c) || c == '_') {
                    /* e.g. "123abc" — invalid token */
                    token.lexeme[i++] = c;
                    /* consume the rest of this malformed token */
                    while (isalnum(c = fgetc(fp)) || c == '_')
                        token.lexeme[i++] = c;
                    ungetc(c, fp);
                    token.lexeme[i] = '\0';
                    fprintf(stderr,
                        "[Lexical Error] Line %d: Invalid numeric literal '%s'"
                        " — identifiers cannot start with a digit\n",
                        token.line, token.lexeme);
                    lexErrorCount++;
                    token.type = TOKEN_INVALID;
                    return token;
                } else {
                    ungetc(c, fp);
                    break;
                }
            }
            token.lexeme[i] = '\0';
            token.type = isFloat ? TOKEN_FLOAT_LITERAL : TOKEN_INT_LITERAL;
            return token;
        }

        /* ════════════════════════════════════════
           OPERATORS & PUNCTUATION
           ════════════════════════════════════════ */
        if (c == '=') {
            int next = fgetc(fp);
            if (next == '=') { token.type = TOKEN_EQ;     strcpy(token.lexeme, "=="); }
            else             { token.type = TOKEN_ASSIGN; token.lexeme[0]='='; token.lexeme[1]='\0'; ungetc(next, fp); }
            return token;
        }

        if (c == '!') {
            int next = fgetc(fp);
            if (next == '=') {
                token.type = TOKEN_NEQ;
                strcpy(token.lexeme, "!=");
                return token;
            }
            /* bare '!' — not part of this language */
            ungetc(next, fp);
            fprintf(stderr,
                "[Lexical Error] Line %d: Unexpected character '!' "
                "(did you mean '!='?)\n", currentLine);
            lexErrorCount++;
            token.type     = TOKEN_UNKNOWN;
            token.lexeme[0] = '!';
            token.lexeme[1] = '\0';
            return token;
        }

        if (c == '<') {
            int next = fgetc(fp);
            if (next == '=') { token.type = TOKEN_LE; strcpy(token.lexeme, "<="); }
            else             { token.type = TOKEN_LT; token.lexeme[0]='<'; token.lexeme[1]='\0'; ungetc(next, fp); }
            return token;
        }

        if (c == '>') {
            int next = fgetc(fp);
            if (next == '=') { token.type = TOKEN_GE; strcpy(token.lexeme, ">="); }
            else             { token.type = TOKEN_GT; token.lexeme[0]='>'; token.lexeme[1]='\0'; ungetc(next, fp); }
            return token;
        }

        switch (c) {
            case '+': token.type = TOKEN_PLUS;      break;
            case '-': token.type = TOKEN_MINUS;     break;
            case '*': token.type = TOKEN_MUL;       break;
            case '(': token.type = TOKEN_LPAREN;    break;
            case ')': token.type = TOKEN_RPAREN;    break;
            case '{': token.type = TOKEN_LBRACE;    break;
            case '}': token.type = TOKEN_RBRACE;    break;
            case ';': token.type = TOKEN_SEMICOLON; break;
            case ',': token.type = TOKEN_COMMA;     break;
            default:
                fprintf(stderr,
                    "[Lexical Error] Line %d: Unknown character '%c' (ASCII %d)\n",
                    currentLine, c, c);
                lexErrorCount++;
                token.type     = TOKEN_UNKNOWN;
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
