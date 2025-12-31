#include "analyzer/use.h"
#include "executor/world.h"
#include "executor/step.h"
#include "analyzer/lifetime.h"
#include "executor/scope.h"



size_t analyze_step_use(
    const World *worlds,
    const ScopeLifetime *lifetimes,
    size_t lifetime_count,
    UseReport *out,
    size_t cap
) {
    size_t n = 0;

    for (const World *w = worlds; w; w = w->next) {
        if (!w->step || w->step->kind != STEP_USE)
            continue;

        if (n >= cap)
            break;

        UseReport *r = &out[n++];
        r->time = w->time;
        r->scope_id = w->active_scope ? w->active_scope->id : 0;
        r->storage_id = w->step->info; /* or UINT64_MAX if unresolved */
        r->kind = USE_OK; /* refined later */
    }

    return n;
}
