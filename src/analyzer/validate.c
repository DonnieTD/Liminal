#include "analyzer/validate.h"

#include "analyzer/trace.h"
#include "executor/world.h"
#include "executor/step.h"

#define MAX_SCOPE_DEPTH 128

size_t validate_scope_invariants(
    struct World *head,
    ScopeViolation *out,
    size_t cap
) {
    uint64_t stack[MAX_SCOPE_DEPTH];
    size_t depth = 0;
    size_t count = 0;

    Trace t = trace_begin(head);
    while (trace_is_valid(&t)) {
        World *w = trace_current(&t);
        if (!w || !w->step) {
            trace_next(&t);
            continue;
        }

        Step *s = w->step;

        if (s->kind == STEP_ENTER_SCOPE) {
            if (depth < MAX_SCOPE_DEPTH) {
                stack[depth++] = s->info;
            }
        }
        else if (s->kind == STEP_EXIT_SCOPE) {
            if (depth == 0) {
                if (count < cap) {
                    out[count++] = (ScopeViolation){
                        .kind = SCOPE_EXIT_WITHOUT_ENTER,
                        .time = w->time,
                        .scope_id = s->info
                    };
                }
            } else {
                uint64_t expected = stack[depth - 1];
                if (expected != s->info) {
                    if (count < cap) {
                        out[count++] = (ScopeViolation){
                            .kind = SCOPE_NON_LIFO_EXIT,
                            .time = w->time,
                            .scope_id = s->info
                        };
                    }
                } else {
                    depth--;
                }
            }
        }

        trace_next(&t);
    }

    /* Unclosed scopes */
    for (size_t i = 0; i < depth && count < cap; i++) {
        out[count++] = (ScopeViolation){
            .kind = SCOPE_ENTER_WITHOUT_EXIT,
            .time = UINT64_MAX,
            .scope_id = stack[i]
        };
    }

    return count;
}
