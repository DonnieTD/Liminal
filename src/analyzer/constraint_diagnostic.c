// src/analyzer/constraint_diagnostic.c
#include "analyzer/constraint.h"
#include "analyzer/diagnostic.h"

size_t constraint_to_diagnostic(
    const ConstraintArtifact *constraints,
    Diagnostic *out,
    size_t cap
) {
    size_t count = 0;

    for (size_t i = 0; i < constraints->count && count < cap; i++) {
        const Constraint *c = &constraints->items[i];

        switch (c->kind) {

        case CONSTRAINT_REDECLARATION:
            out[count++] = (Diagnostic){
                .kind = DIAG_REDECLARATION,
                .time = c->time,
                .scope_id = c->scope_id,
                .previous_scope_id = c->scope_id,
                .name = NULL,
                .origin = NULL,
                .previous_origin = NULL
            };
            break;

        case CONSTRAINT_SHADOWING:
            out[count++] = (Diagnostic){
                .kind = DIAG_SHADOWING,
                .time = c->time,
                .scope_id = c->scope_id,
                .previous_scope_id = 0, /* parent scope not surfaced yet */
                .name = NULL,
                .origin = NULL,
                .previous_origin = NULL
            };
            break;

        case CONSTRAINT_USE_REQUIRES_DECLARATION:
            out[count++] = (Diagnostic){
                .kind = DIAG_USE_BEFORE_DECLARE,
                .time = c->time,
                .scope_id = c->scope_id,
                .previous_scope_id = 0,
                .name = NULL,
                .origin = NULL,
                .previous_origin = NULL
            };
            break;

        default:
            /* Unknown / future constraint — ignored by design */
            break;
        }
    }

    return count;
}
