#include "./ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


ASTProgram *ast_program_new(const char *path,
                            const char *src,
                            size_t len)
{
    ASTProgram *p = calloc(1, sizeof(ASTProgram));
    if (!p) return NULL;

    p->nodes = NULL;
    p->count = 0;
    p->cap   = 0;

    p->source_path = strdup(path);
    p->source_text = strdup(src);
    p->source_len  = len;

    p->root_id = 0; /* IMPORTANT */

    return p;
}


void ast_program_free(ASTProgram *p)
{
    if (!p) return;

    /* Free block stmt arrays */
    for (size_t i = 0; i < p->count; i++) {
        ASTNode *n = &p->nodes[i];
        if (n->kind == AST_BLOCK) {
            free(n->as.block.stmt_ids);
            n->as.block.stmt_ids = NULL;
            n->as.block.stmt_count = 0;
        }
    }

    free(p->nodes);
    free(p->source_path);
    free(p->source_text);
    free(p);
}

ASTNode *ast_node_get(ASTProgram *p, uint32_t id)
{
    if (!p || id == 0) return NULL;
    size_t idx = (size_t)(id - 1);
    if (idx >= p->count) return NULL;
    return &p->nodes[idx];
}

uint32_t ast_add_node(ASTProgram *p, ASTKind kind, ASTSpan at)
{
    if (!p) return 0;

    if (p->count >= p->cap) {
        size_t ncap = (p->cap == 0) ? 8 : p->cap * 2;
        ASTNode *nn = (ASTNode *)realloc(p->nodes, ncap * sizeof(ASTNode));
        if (!nn) return 0;
        /* zero new range */
        memset(nn + p->cap, 0, (ncap - p->cap) * sizeof(ASTNode));
        p->nodes = nn;
        p->cap = ncap;
    }

    ASTNode *n = &p->nodes[p->count];
    memset(n, 0, sizeof(*n));
    n->id = (uint32_t)(p->count + 1);
    n->kind = kind;
    n->at = at;

    p->count++;
    return n->id;
}
