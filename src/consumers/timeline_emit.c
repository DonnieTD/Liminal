#include <stdio.h>

#include "consumers/timeline_emit.h"
#include "executor/world.h"
#include "executor/step.h"
#include "frontends/c/ast.h"

/*
 * Emit execution timeline as NDJSON
 *
 * NOTE:
 *  - origin is opaque at executor level
 *  - consumers may interpret it
 */
void timeline_emit_ndjson(
    const struct World *head,
    FILE *out
)
{
    const struct World *w = head;

    while (w) {
        uint32_t ast_id = 0;

        if (w->step && w->step->origin) {
            const ASTNode *n = (const ASTNode *)w->step->origin;
            ast_id = n->id;
        }

        fprintf(
            out,
            "{\"time\":%llu,\"step\":%d,\"ast\":%u}\n",
            (unsigned long long)w->time,
            w->step ? w->step->kind : 0,
            ast_id
        );

        w = w->next;
    }
}

/*
 * Emit human-readable timeline
 */
void emit_timeline(
    const struct World *head,
    FILE *out
)
{
    const struct World *w = head;

    while (w) {
        uint32_t ast_id = 0;

        if (w->step && w->step->origin) {
            const ASTNode *n = (const ASTNode *)w->step->origin;
            ast_id = n->id;
        }

        fprintf(
            out,
            "t=%llu step=%d ast=%u\n",
            (unsigned long long)w->time,
            w->step ? w->step->kind : 0,
            ast_id
        );

        w = w->next;
    }
}
