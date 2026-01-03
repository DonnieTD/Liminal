#include "./lexer.h"
#include <string.h>
#include <ctype.h>

void lexer_init(Lexer *lx, const char *path, const char *src, size_t len)
{
    lx->path = path;
    lx->src  = src;
    lx->len  = len;
    lx->pos  = 0;
}

static void skip_ws(Lexer *lx)
{
    while (lx->pos < lx->len && isspace((unsigned char)lx->src[lx->pos])) {
        lx->pos++;
    }
}

Token lexer_next(Lexer *lx)
{
    skip_ws(lx);

    if (lx->pos >= lx->len) {
        return (Token){ TOK_EOF, NULL, 0 };
    }

    const char *s = lx->src + lx->pos;

    /* Keywords */
    if (strncmp(s, "int", 3) == 0 && !isalnum(s[3])) {
        lx->pos += 3;
        return (Token){ TOK_INT, s, 3 };
    }

    if (strncmp(s, "return", 6) == 0 && !isalnum(s[6])) {
        lx->pos += 6;
        return (Token){ TOK_RETURN, s, 6 };
    }

    /* Identifier (only "main" supported) */
    if (isalpha((unsigned char)*s)) {
        size_t i = 0;
        while (isalnum((unsigned char)s[i])) i++;
        lx->pos += i;
        return (Token){ TOK_IDENT, s, i };
    }

    /* Integer literal */
    if (isdigit((unsigned char)*s)) {
        size_t i = 0;
        while (isdigit((unsigned char)s[i])) i++;
        lx->pos += i;
        return (Token){ TOK_INT_LIT, s, i };
    }

    /* Punctuation */
    lx->pos++;
    switch (*s) {
    case '(': return (Token){ TOK_LPAREN, s, 1 };
    case ')': return (Token){ TOK_RPAREN, s, 1 };
    case '{': return (Token){ TOK_LBRACE, s, 1 };
    case '}': return (Token){ TOK_RBRACE, s, 1 };
    case ';': return (Token){ TOK_SEMI,   s, 1 };
    default:  return (Token){ TOK_EOF, NULL, 0 };
    }
}

int lexer_accept(Lexer *lx, TokKind k)
{
    size_t save = lx->pos;
    Token t = lexer_next(lx);
    if (t.kind == k) return 1;
    lx->pos = save;
    return 0;
}
