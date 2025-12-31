#include <stdlib.h>
#include <string.h>
#include "frontends/c/parser.h"

/*
 * Parse a C translation unit.
 *
 * STAGE 1.5:
 *   - minimal real parser
 *   - supports:
 *       int main() { <stmts> }
 *       int <ident> ;
 *       return <int> ;
 *
 * Produces a structural AST artifact only.
 */
ASTProgram *parse_translation_unit(Lexer *lx)
{
    ASTProgram *p = ast_program_new(lx->path, lx->src, lx->len);
    if (!p) return NULL;

    /* dummy span for now */
    ASTSpan z = { .line = 1, .col = 1 };

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

    /* ---- Parse statements ---- */

    uint32_t stmts[16];
    size_t stmt_count = 0;

    for (;;) {
        /* End of block */
        if (lexer_accept(lx, TOK_RBRACE)) {
            break;
        }

        /* int <ident> ; */
        if (lexer_accept(lx, TOK_INT)) {
            Token id = lexer_next(lx);
            if (id.kind != TOK_IDENT) goto fail;
            if (!lexer_accept(lx, TOK_SEMI)) goto fail;

            uint32_t node_id = ast_add_node(p, AST_VAR_DECL, z);
            ASTNode *vd = ast_node_get(p, node_id);
            vd->as.vdecl.name = strndup(id.lexeme, id.len);

            stmts[stmt_count++] = node_id;
            continue;
        }

        /* return <int> ; */
        if (lexer_accept(lx, TOK_RETURN)) {
            Token lit = lexer_next(lx);
            if (lit.kind != TOK_INT_LIT) goto fail;
            if (!lexer_accept(lx, TOK_SEMI)) goto fail;

            uint32_t node_id = ast_add_node(p, AST_RETURN, z);
            ASTNode *r = ast_node_get(p, node_id);
            r->as.ret.value = 0; /* literal ignored for now */

            stmts[stmt_count++] = node_id;
            continue;
        }

        /* <ident> ;  → variable use */
        {
            size_t save = lx->pos;
            Token id = lexer_next(lx);

            if (id.kind == TOK_IDENT) {
                if (lexer_accept(lx, TOK_SEMI)) {
                    uint32_t node_id = ast_add_node(p, AST_VAR_USE, z);
                    ASTNode *vu = ast_node_get(p, node_id);
                    vu->as.vuse.name = strndup(id.lexeme, id.len);

                    stmts[stmt_count++] = node_id;
                    continue;
                }
            }

            lx->pos = save;
        }

        goto fail;
    }

    /* ---- Build structural AST ---- */

    uint32_t prog_id = ast_add_node(p, AST_PROGRAM, z);
    uint32_t fn_id   = ast_add_node(p, AST_FUNCTION, z);
    uint32_t blk_id  = ast_add_node(p, AST_BLOCK, z);

    ASTNode *fn  = ast_node_get(p, fn_id);
    ASTNode *blk = ast_node_get(p, blk_id);

    /* function name */
    {
        const char *name = "main";
        size_t len = strlen(name) + 1;
        char *owned = malloc(len);
        if (!owned) goto fail;
        memcpy(owned, name, len);
        fn->as.fn.name = owned;
    }

    fn->as.fn.body_id = blk_id;

    blk->as.block.stmt_ids = malloc(sizeof(uint32_t) * stmt_count);
    if (!blk->as.block.stmt_ids) goto fail;

    memcpy(blk->as.block.stmt_ids, stmts, sizeof(uint32_t) * stmt_count);
    blk->as.block.stmt_count = stmt_count;

    p->root_id = prog_id;
    return p;

fail:
    ast_program_free(p);
    return NULL;
}
