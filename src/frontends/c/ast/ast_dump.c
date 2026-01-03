#include <stdio.h>
#include "ast.h"
#include "../../../analyzer/analyzer.h"
/*
 * AST dump
 *
 * This emits the FRONTEND ARTIFACT.
 *
 * - No execution
 * - No analysis
 * - No interpretation
 *
 * This is a structural, inspectable snapshot of the AST.
 */


void ast_dump(const ASTProgram *p)
{
    printf("\n-- AST ARTIFACT --\n");

    if (!p || p->count == 0) {
        printf("node_count=0\nroot_id=0\n(empty AST)\n");
        return;
    }

    if (p->root_id == 0) {
        printf("(invalid AST: no root)\n");
        return;
    }

    printf("node_count=%zu\n", p->count);
    printf("root_id=%u\n\n", p->root_id);

    for (size_t i = 0; i < p->count; i++) {
        const ASTNode *n = &p->nodes[i];

        printf("[%u] ", n->id);

        switch (n->kind) {
        case AST_PROGRAM:
            printf("PROGRAM\n");
            break;

        case AST_FUNCTION:
            printf("FUNCTION name=%s body=%u\n",
                n->as.fn.name,
                n->as.fn.body_id);
            break;

        case AST_BLOCK:
            printf("BLOCK stmts=%zu\n",
                n->as.block.stmt_count);
            break;

        case AST_RETURN:
            printf("RETURN value=%lld\n",
                (long long)n->as.ret.value);
            break;

        default:
            printf("UNKNOWN\n");
        }
    }
}
