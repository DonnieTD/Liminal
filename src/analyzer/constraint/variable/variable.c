#include "analyzer/analyzer.h"
#include "executor/executor.h"

#include <stdlib.h>
#include <stdint.h>

ConstraintArtifact analyze_variable_constraints(struct World *head)
{
    /* Empty artifact for degenerate cases */
    if (!head) {
        return (ConstraintArtifact){
            .items = NULL,
            .count = 0
        };
    }

    /* Fixed-cap temporary buffer (Stage 4.x discipline) */
    size_t cap = 64;
    Constraint *buf = calloc(cap, sizeof(Constraint));
    size_t count = 0;

    if (!buf) {
        return (ConstraintArtifact){
            .items = NULL,
            .count = 0
        };
    }

    Trace t = trace_begin(head);
    while (trace_is_valid(&t)) {
        World *w = trace_current(&t);
        Step  *s = w ? w->step : NULL;

        if (s && s->kind == STEP_USE) {
            /* Unresolved variable use → constraint */
            if (s->info == UINT64_MAX && count < cap) {
                buf[count++] = (Constraint){
                    .kind       = CONSTRAINT_USE_REQUIRES_DECLARATION,
                    .time       = w->time,
                    .scope_id   = 0,           /* scope not required yet */
                    .storage_id = UINT64_MAX
                };
            }
        }

        trace_next(&t);
    }

    return (ConstraintArtifact){
        .items = buf,
        .count = count
    };
}
