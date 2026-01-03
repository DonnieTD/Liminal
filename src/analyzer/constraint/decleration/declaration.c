//@source src/analyzer/constraint_declaration.c
#include "analyzer/analyzer.h"
#include "executor/executor.h"
#include "common/common.h"
#include "frontends/frontends.h"   /* for ASTNode */
#include <stdlib.h>
#include <stdint.h>

ConstraintArtifact analyze_declaration_constraints(struct World *head)
{
    size_t cap = 64;
    Constraint *buf = calloc(cap, sizeof(Constraint));
    size_t count = 0;

    if (!buf || !head) {
        return (ConstraintArtifact){ .items = NULL, .count = 0 };
    }

    Trace t = trace_begin(head);

    while (trace_is_valid(&t)) {
        World *w = trace_current(&t);
        Step  *s = w ? w->step : NULL;

        if (!s || s->kind != STEP_DECLARE) {
            trace_next(&t);
            continue;
        }

        World *prev = w->prev;
        if (!prev || !prev->active_scope)
            goto next;

        Scope *cur = prev->active_scope;
        const char *name = NULL;

        /* Extract name from AST origin (safe for now) */
        if (s->origin) {
            ASTNode *n = (ASTNode *)s->origin;
            name = n->as.vdecl.name;
        }

        if (!name)
            goto next;

        /* 1. Redeclaration in same scope */
        if (scope_has_name(cur, name) && count < cap) {
            buf[count++] = (Constraint){
                .kind       = CONSTRAINT_REDECLARATION,
                .time       = w->time,
                .scope_id   = cur->id,
                .storage_id = s->info,
                .anchor     = anchor_from_origin(s->origin)
            };
            goto next;
        }

        /* 2. Shadowing parent scope */
        for (Scope *p = cur->parent; p; p = p->parent) {
            if (scope_has_name(p, name) && count < cap) {
                buf[count++] = (Constraint){
                    .kind       = CONSTRAINT_SHADOWING,
                    .time       = w->time,
                    .scope_id   = cur->id,
                    .storage_id = s->info,
                    .anchor     = anchor_from_origin(s->origin)
                };
                break;
            }
        }

    next:
        trace_next(&t);
    }

    return (ConstraintArtifact){
        .items = buf,
        .count = count
    };
}
