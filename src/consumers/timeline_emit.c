#include <stdio.h>
#include <stdint.h>

#include "consumers/timeline_emit.h"
#include "executor/world.h"
#include "executor/step.h"
#include "frontends/c/ast.h"

/*
 * Timeline NDJSON — Stage 7 canonical artifact
 *
 * Contract:
 *  - One line per World
 *  - Deterministic
 *  - No executor pointers
 *  - No formatting variance
 *  - Stable across runs
 *
 * Schema v1:
 * { "v":1, "t":<uint64>, "step":"<name>", "ast":<uint32> }
 */
void timeline_emit_ndjson(
    const struct World *head,
    FILE *out
)
{
    const struct World *w = head;

    while (w) {
        uint32_t ast_id = 0;
        const char *step_name = "UNKNOWN";

        if (w->step) {
            step_name = step_kind_name(w->step->kind);

            if (w->step->origin) {
                const ASTNode *n =
                    (const ASTNode *)w->step->origin;
                ast_id = n->id;
            }
        }

        fprintf(
            out,
            "{\"v\":1,\"t\":%llu,\"step\":\"%s\",\"ast\":%u}\n",
            (unsigned long long)w->time,
            step_name,
            ast_id
        );

        w = w->next;
    }
}

/*
 * Human-readable timeline (NON-CANONICAL)
 *
 * Debug / inspection only.
 * NOT used for diffing or artifacts.
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
