#include "analyzer/use.h"
#include "analyzer/trace.h"
#include "executor/world.h"
#include "executor/step.h"
#include "lifetime.h"
#include "executor/scope.h"

size_t analyze_step_use(
    const struct World *worlds,
    const struct ScopeLifetime *lifetimes,
    size_t lifetime_count,
    UseReport *out,
    size_t cap
) {
    size_t count = 0;

    Trace t = trace_begin((struct World *)worlds);
    while (trace_is_valid(&t)) {
        const World *w = trace_current(&t);
        const Step *s = w->step;

        if (!s || s->kind != STEP_USE)
            goto next;

        if (count >= cap)
            break;

        UseReport r = {
            .time = w->time,
            .scope_id = w->active_scope ? w->active_scope->id : 0,
            .storage_id = s->info,
            .kind = USE_OK
        };

        /* Rule 1: use before declaration */
        if (s->info == UINT64_MAX) {
            r.kind = USE_BEFORE_DECLARE;
            out[count++] = r;
            goto next;
        }

        /* Rule 2: use after scope exit */
        for (size_t i = 0; i < lifetime_count; i++) {
            const ScopeLifetime *lt = &lifetimes[i];
            if (lt->scope_id == r.scope_id &&
                lt->exit_time != UINT64_MAX &&
                r.time > lt->exit_time) {
                r.kind = USE_AFTER_SCOPE;
                out[count++] = r;
                goto next;
            }
        }

    next:
        trace_next(&t);
    }

    return count;
}
