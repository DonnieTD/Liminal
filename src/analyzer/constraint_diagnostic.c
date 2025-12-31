#include "analyzer/constraint.h"
#include "analyzer/constraint_diagnostic.h"
#include "analyzer/diagnostic.h"
#include "analyzer/diagnostic_id.h"

size_t constraint_to_diagnostic(
    const ConstraintArtifact *constraints,
    Diagnostic *out,
    size_t cap
) {
    size_t count = 0;

    if (!constraints || !out || cap == 0)
        return 0;

    for (size_t i = 0; i < constraints->count && count < cap; i++) {
        const Constraint *c = &constraints->items[i];
        Diagnostic *d = &out[count];

        /* Stable identity derived from constraint */
        d->id = diagnostic_id_from_constraint(c);

        d->time      = c->time;
        d->scope_id  = c->scope_id;
        d->prev_scope = 0;
        d->anchor    = c->anchor;

        switch (c->kind) {

        case CONSTRAINT_REDECLARATION:
            d->kind = DIAG_REDECLARATION;
            d->prev_scope = c->scope_id;
            break;

        case CONSTRAINT_SHADOWING:
            d->kind = DIAG_SHADOWING;
            /* parent scope not yet surfaced */
            d->prev_scope = 0;
            break;

        case CONSTRAINT_USE_REQUIRES_DECLARATION:
            d->kind = DIAG_USE_BEFORE_DECLARE;
            break;

        default:
            /* Unknown / future constraint — ignored by design */
            continue;
        }

        count++;
    }

    return count;
}
