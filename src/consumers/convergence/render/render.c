#include <stdio.h>

#include "../map/map.h"
#include "../../../executor/executor.h"
#include "analyzer/analyzer.h"

/*
 * render_convergence
 *
 * Presentation-only.
 * No building, no allocation, no mutation.
 */
void render_convergence(const ConvergenceMap *m)
{
    if (!m || m->count == 0)
        return;

    printf("\n== Cross-Diagnostic Convergence ==\n");

    for (size_t i = 0; i < m->count; i++) {
        const ConvergenceEntry *e = &m->entries[i];

        /* Only interesting if multiple diagnostics converge */
        if (e->count < 2)
            continue;

        printf(
            "\nCAUSE: step=%s ast=%llu scope=%llu\n",
            step_kind_name(e->key.step),
            (unsigned long long)e->key.ast_id,
            (unsigned long long)e->key.scope_id
        );

        for (size_t j = 0; j < e->count; j++) {
            const Diagnostic *d = e->diagnostics[j];
            printf(
                "  ↳ %s (time=%llu)\n",
                diagnostic_kind_name(d->kind),
                (unsigned long long)d->time
            );
        }
    }
}
