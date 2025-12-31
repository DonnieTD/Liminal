#include <stdlib.h>
#include <string.h>
#include "frontends/c/parser.h"

/*
 * Parse a C translation unit.
 *
 * STAGE 1:
 *   - no real parsing
 *   - returns a minimal ASTProgram artifact
 *
 * This satisfies the frontend → executor contract.
 */
ASTProgram *parse_translation_unit(Lexer *lx)
{
    ASTProgram *p = ast_program_new(lx->path, lx->src, lx->len);
    if (!p) return NULL;

    /* We assume lexer already skipped whitespace/comments */

    /* Expect: int */
    if (!lexer_accept(lx, TOK_INT)) goto fail;

    /* Expect: main */
    Token ident = lexer_next(lx);
    if (ident.kind != TOK_IDENT) goto fail;

    /* Expect: () */
    if (!lexer_accept(lx, TOK_LPAREN)) goto fail;
    if (!lexer_accept(lx, TOK_RPAREN)) goto fail;

    /* Expect: { */
    if (!lexer_accept(lx, TOK_LBRACE)) goto fail;

    /* Expect: return */
    if (!lexer_accept(lx, TOK_RETURN)) goto fail;

    /* Expect: integer literal */
    Token lit = lexer_next(lx);
    if (lit.kind != TOK_INT_LIT) goto fail;

    /* Expect: ; } */
    if (!lexer_accept(lx, TOK_SEMI)) goto fail;
    if (!lexer_accept(lx, TOK_RBRACE)) goto fail;

    /* ---- Build AST ---- */

    ASTSpan z = { .line = 1, .col = 1 };

    uint32_t prog_id = ast_add_node(p, AST_PROGRAM, z);
    uint32_t fn_id   = ast_add_node(p, AST_FUNCTION, z);
    uint32_t blk_id  = ast_add_node(p, AST_BLOCK, z);
    uint32_t ret_id  = ast_add_node(p, AST_RETURN, z);

    ASTNode *fn  = ast_node_get(p, fn_id);
    ASTNode *blk = ast_node_get(p, blk_id);
    ASTNode *ret = ast_node_get(p, ret_id);

    {
        const char *name = "main";
        size_t len = strlen(name) + 1;

        char *owned = malloc(len);
        if (!owned) goto fail;

        memcpy(owned, name, len);
        fn->as.fn.name = owned;
    }

    fn->as.fn.body_id    = blk_id;

    blk->as.block.stmt_ids   = malloc(sizeof(uint32_t));
    blk->as.block.stmt_ids[0] = ret_id;
    blk->as.block.stmt_count = 1;

    ret->as.ret.value = 0;

    p->root_id = prog_id;
    return p;

fail:
    ast_program_free(p);
    return NULL;
}
