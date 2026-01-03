#include "./render.h"

#include <stdio.h>

#include "analyzer/analyzer.h"

/*
 * Terminal renderer for diagnostics.
 *
 * Stage 5.4 discipline:
 *  - read-only
 *  - no allocation
 *  - no formatting state
 *  - stable output
 */

void
diagnostic_render_terminal(
    const DiagnosticArtifact *a,
    FILE *out
) {
    if (!a || !a->items || a->count == 0) {
        fprintf(out, "(no diagnostics)\n");
        return;
    }

    for (size_t i = 0; i < a->count; i++) {
        const Diagnostic *d = &a->items[i];

        fprintf(
            out,
            "[%02zu] %-24s time=%llu scope=%llu",
            i,
            diagnostic_kind_name(d->kind),
            (unsigned long long)d->time,
            (unsigned long long)d->scope_id
        );

        if (d->prev_scope) {
            fprintf(
                out,
                " prev_scope=%llu",
                (unsigned long long)d->prev_scope
            );
        }

        fprintf(
            out,
            " id=%016llx",
            (unsigned long long)d->id.value
        );

        if (d->anchor) {
            fprintf(
                out,
                " @ node=%u:%u:%u",
                d->anchor->node_id,
                d->anchor->line,
                d->anchor->col
            );
        }

        fputc('\n', out);
    }
}
