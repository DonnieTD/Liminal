#include "analyzer/analyzer.h"
#include "executor/executor.h"


size_t lifetime_collect_variables(
    struct World *head,
    VariableLifetime *out,
    size_t cap
) {
    size_t n = 0;
    Trace t = trace_begin(head);

    while (trace_is_valid(&t)) {
        World *w = trace_current(&t);
        Step *s = w->step;

        if (s && s->kind == STEP_DECLARE) {
            if (n >= cap) break;

            out[n++] = (VariableLifetime){
                .var_id        = s->info,
                .scope_id      = w->active_scope ? w->active_scope->id : 0,
                .declare_time  = w->time,
                .end_time      = UINT64_MAX
            };
        }

        if (s && s->kind == STEP_EXIT_SCOPE) {
            uint64_t sid = s->info;
            for (size_t i = 0; i < n; i++) {
                if (out[i].scope_id == sid &&
                    out[i].end_time == UINT64_MAX) {
                    out[i].end_time = w->time;
                }
            }
        }

        trace_next(&t);
    }

    return n;
}
