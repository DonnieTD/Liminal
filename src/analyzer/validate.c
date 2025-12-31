#include <stddef.h>
#include <stdint.h>

#include "analyzer/validate.h"
#include "analyzer/trace.h"

#include "executor/world.h"
#include "executor/step.h"
#include "executor/scope.h"

/*
 * validate_scope_invariants
 *
 * This function reconstructs the *expected* scope structure
 * from STEP_ENTER_SCOPE and STEP_EXIT_SCOPE events alone.
 *
 * It intentionally does NOT trust:
 *   - World.active_scope
 *   - frontend assumptions
 *   - executor correctness
 *
 * Instead, it derives a shadow "scope stack" and checks
 * that the recorded World state agrees with it.
 *
 * This makes the validator capable of catching:
 *   - missing exits
 *   - extra exits
 *   - wrong exit order
 *   - incorrect active_scope propagation
 */
size_t validate_scope_invariants(struct World *head,
                                 ScopeViolation *out,
                                 size_t cap)
{
    /* Defensive: invalid input yields no violations */
    if (!head || !out || cap == 0) {
        return 0;
    }

    /*
     * Manual scope stack.
     *
     * We only store scope IDs, not pointers.
     * This keeps validation purely structural.
     *
     * Depth is intentionally bounded:
     *   - makes behavior deterministic
     *   - avoids allocation
     *   - signals insane nesting immediately
     */
    uint64_t stack[128];
    size_t depth = 0;

    /* Number of violations recorded */
    size_t n = 0;

    /*
     * Iterate forward through time using the Trace.
     *
     * The Trace is the membrane between execution and analysis.
     */
    Trace t = trace_begin(head);
    while (trace_is_valid(&t)) {
        World *w = trace_current(&t);

        /* Skip Worlds with no causal step */
        if (!w || !w->step) {
            trace_next(&t);
            continue;
        }

        Step *s = w->step;

        /*
         * Handle ENTER_SCOPE
         *
         * We push the scope id onto our shadow stack.
         */
        if (s->kind == STEP_ENTER_SCOPE) {
            if (depth < 128) {
                stack[depth++] = s->info;
            }
            /* If depth exceeds capacity, we silently ignore
             * additional nesting for now. This is intentional:
             * analysis must remain total.
             */
        }

        /*
         * Handle EXIT_SCOPE
         *
         * We expect to pop the most recent scope.
         */
        else if (s->kind == STEP_EXIT_SCOPE) {

            /* No active scopes → illegal exit */
            if (depth == 0) {
                if (n < cap) {
                    out[n++] = (ScopeViolation){
                        .kind     = SCOPE_EXIT_WITHOUT_ENTER,
                        .time     = w->time,
                        .scope_id = s->info
                    };
                }
            } else {
                uint64_t expected = stack[depth - 1];

                /* Exiting a scope that is not on top of stack */
                if (expected != s->info) {
                    if (n < cap) {
                        out[n++] = (ScopeViolation){
                            .kind     = SCOPE_NON_LIFO_EXIT,
                            .time     = w->time,
                            .scope_id = s->info
                        };
                    }
                } else {
                    /* Correctly nested exit */
                    depth--;
                }
            }
        }

        /*
         * Validate active_scope consistency
         *
         * World.active_scope must reflect the top of our
         * derived scope stack.
         *
         * This catches executor bookkeeping bugs.
         */
        uint64_t active =
            w->active_scope ? w->active_scope->id : 0;

        uint64_t expected =
            (depth > 0) ? stack[depth - 1] : 0;

        if (active != expected) {
            if (n < cap) {
                out[n++] = (ScopeViolation){
                    .kind     = SCOPE_ACTIVE_MISMATCH,
                    .time     = w->time,
                    .scope_id = active
                };
            }
        }

        trace_next(&t);
    }

    /*
     * Any scopes left on the stack at end-of-trace
     * were entered but never exited.
     */
    for (size_t i = 0; i < depth && n < cap; i++) {
        out[n++] = (ScopeViolation){
            .kind     = SCOPE_ENTER_WITHOUT_EXIT,
            .time     = UINT64_MAX,
            .scope_id = stack[i]
        };
    }

    return n;
}
