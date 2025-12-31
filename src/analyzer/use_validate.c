#include "analyzer/use_validate.h"
#include "analyzer/use.h"
#include "analyzer/lifetime.h"
#include "executor/world.h"

size_t analyze_use_validation(
    struct World *head,
    Diagnostic *out,
    size_t cap
) {
    ScopeLifetime lifetimes[128];
    size_t lt_count = lifetime_collect_scopes(head, lifetimes, 128);

    UseReport uses[128];
    size_t use_count = analyze_step_use(
        head,
        lifetimes,
        lt_count,
        uses,
        128
    );

    size_t count = 0;
    for (size_t i = 0; i < use_count && count < cap; i++) {
        const UseReport *u = &uses[i];

        if (u->kind == USE_OK)
            continue;

        out[count++] = (Diagnostic){
            .kind =
                (u->kind == USE_BEFORE_DECLARE)
                    ? DIAG_USE_BEFORE_DECLARE
                    : DIAG_USE_AFTER_SCOPE_EXIT,
            .time = u->time,
            .scope_id = u->scope_id,
            .previous_scope_id = 0,
            .name = NULL,
            .origin = NULL,
            .previous_origin = NULL
        };
    }

    return count;
}
