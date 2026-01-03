#include "./cause.h"
#include "../../../executor/executor.h"
#include "../../../frontends/frontends.h"
#include "../../../analyzer/analyzer.h"

RootCause root_cause_extract(
    const struct World *head,
    const struct Diagnostic *d
)
{
    const struct World *w = head;

    /* Seek to diagnostic time */
    while (w && w->time != d->time)
        w = w->next;

    /* Walk backwards */
    while (w && w->prev) {
        w = w->prev;
        if (!w->step)
            continue;

        ASTNode *n = (ASTNode *)w->step->origin;
        uint64_t ast_id = n ? n->id : 0;

        /* Declaration-related diagnostics */
        if (d->kind == DIAG_REDECLARATION ||
            d->kind == DIAG_SHADOWING) {

            if (w->step->kind == STEP_DECLARE) {
                return (RootCause){
                    .kind     = ROOT_CAUSE_DECLARATION,
                    .time     = w->time,
                    .ast_id   = ast_id,
                    .scope_id = d->scope_id   /* diagnostic-derived */
                };
            }
        }

        /* Use-related diagnostics */
        if (d->kind == DIAG_USE_BEFORE_DECLARE) {
            if (w->step->kind == STEP_USE) {
                return (RootCause){
                    .kind     = ROOT_CAUSE_USE,
                    .time     = w->time,
                    .ast_id   = ast_id,
                    .scope_id = d->scope_id
                };
            }
        }

        /* Scope entry / exit is authoritative */
        if (w->step->kind == STEP_ENTER_SCOPE ||
            w->step->kind == STEP_EXIT_SCOPE) {

            return (RootCause){
                .kind     = (w->step->kind == STEP_ENTER_SCOPE)
                              ? ROOT_CAUSE_SCOPE_ENTRY
                              : ROOT_CAUSE_SCOPE_EXIT,
                .time     = w->time,
                .ast_id   = ast_id,
                .scope_id = w->step->info   /* ← THIS is correct */
            };
        }
    }

    return (RootCause){
        .kind     = ROOT_CAUSE_UNKNOWN,
        .time     = d->time,
        .ast_id   = 0,
        .scope_id = d->scope_id
    };
}
