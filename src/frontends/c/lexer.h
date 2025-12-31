#ifndef LIMINAL_C_LEXER_H
#define LIMINAL_C_LEXER_H

#include <stddef.h>

typedef enum TokKind {
    TOK_EOF = 0,

    /* Keywords */
    TOK_INT,
    TOK_RETURN,

    /* Identifiers / literals */
    TOK_IDENT,
    TOK_INT_LIT,

    /* Punctuation */
    TOK_LPAREN,   /* ( */
    TOK_RPAREN,   /* ) */
    TOK_LBRACE,   /* { */
    TOK_RBRACE,   /* } */
    TOK_SEMI      /* ; */
} TokKind;

typedef struct Token {
    TokKind kind;
    const char *lexeme;   /* points into source */
    size_t len;
} Token;

typedef struct Lexer {
    const char *path;
    const char *src;
    size_t      len;
    size_t      pos;
} Lexer;

/* API */
void  lexer_init(Lexer *lx, const char *path, const char *src, size_t len);
Token lexer_next(Lexer *lx);
int   lexer_accept(Lexer *lx, TokKind k);

#endif /* LIMINAL_C_LEXER_H */
