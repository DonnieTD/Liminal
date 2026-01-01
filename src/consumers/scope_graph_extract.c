#include "consumers/scope_graph_extract.h"
#include "executor/step.h"
#include "executor/scope.h"  
#include <stdlib.h>

ScopeGraph scope_graph_extract(
    const struct World *head
)
{
    ScopeNode *buf = calloc(128, sizeof(ScopeNode));
    size_t count = 0;

    const struct World *w = head;

    while (w) {
        if (w->step) {
            if (w->step->kind == STEP_ENTER_SCOPE) {
                ScopeNode *n = &buf[count++];
                n->scope_id   = w->step->info;
                n->parent_id  =
                    w->prev && w->prev->active_scope
                        ? w->prev->active_scope->id
                        : 0;
                n->enter_time = w->time;
                n->exit_time  = UINT64_MAX;
            }

            if (w->step->kind == STEP_EXIT_SCOPE) {
                uint64_t sid = w->step->info;
                for (size_t i = count; i > 0; i--) {
                    if (buf[i - 1].scope_id == sid &&
                        buf[i - 1].exit_time == UINT64_MAX) {
                        buf[i - 1].exit_time = w->time;
                        break;
                    }
                }
            }
        }
        w = w->next;
    }

    return (ScopeGraph){
        .nodes = buf,
        .count = count
    };
}
