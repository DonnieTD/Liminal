#include <stdint.h>
#include <stddef.h>

#include "analyzer/analyzer.h"

#include "executor/executor.h"

/*
 * We derive scope lifetimes purely from Steps:
 *  - STEP_ENTER_SCOPE: open lifetime
 *  - STEP_EXIT_SCOPE : close lifetime
 *
 * Step->info carries the scope id for both enter and exit.
 */
size_t lifetime_collect_scopes(struct World *head,
                               ScopeLifetime *out,
                               size_t cap)
{
    if (!head || !out || cap == 0) {
        return 0;
    }

    size_t n = 0;

    Trace t = trace_begin(head);
    while (trace_is_valid(&t)) {
        World *w = trace_current(&t);
        if (!w || !w->step) {
            trace_next(&t);
            continue;
        }

        Step *s = w->step;

        if (s->kind == STEP_ENTER_SCOPE) {
            if (n >= cap) {
                return n; /* truncate for now */
            }

            out[n].scope_id      = s->info;
            out[n].enter_time    = w->time;
            out[n].exit_time     = UINT64_MAX;
            out[n].enter_origin  = s->origin;
            out[n].exit_origin   = NULL;
            n++;
        } else if (s->kind == STEP_EXIT_SCOPE) {
            /* close the most recent open lifetime with matching scope_id */
            uint64_t sid = s->info;

            for (size_t i = n; i > 0; i--) {
                ScopeLifetime *lt = &out[i - 1];
                if (lt->scope_id == sid && lt->exit_time == UINT64_MAX) {
                    lt->exit_time   = w->time;
                    lt->exit_origin = s->origin;
                    break;
                }
            }
        }

        trace_next(&t);
    }

    return n;
}
