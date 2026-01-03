#include "consumers/consumers.h"
#include "executor/executor.h"
#include "frontends/frontends.h"   /* for ASTNode */

size_t timeline_extract(
    const struct World *head,
    TimelineEvent *out,
    size_t cap
)
{
    size_t count = 0;
    const struct World *w = head;

    while (w && count < cap) {
        uint32_t ast_id = 0;
        if (w->step && w->step->origin) {
            const ASTNode *n = (const ASTNode *)w->step->origin;
            ast_id = n->id;
        }

        out[count++] = (TimelineEvent){
            .time = w->time,
            .step_kind = w->step ? w->step->kind : 0,
            .ast_id = ast_id
        };

        w = w->next;
    }

    return count;
}
