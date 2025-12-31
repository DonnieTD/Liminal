#include "analyzer/diagnostic.h"
#include "analyzer/diagnostic_id.h"
#include <stdio.h>

static const char *kind_str(DiagnosticKind k)
{
    switch (k) {
    case DIAG_REDECLARATION:        return "REDECLARATION";
    case DIAG_SHADOWING:            return "SHADOWING";
    case DIAG_USE_BEFORE_DECLARE:   return "USE_BEFORE_DECLARE";
    case DIAG_USE_AFTER_SCOPE_EXIT: return "USE_AFTER_SCOPE_EXIT";
    default:                        return "UNKNOWN";
    }
}

void diagnostic_dump(const DiagnosticArtifact *a)
{
    if (!a || a->count == 0)
        return;

    printf("\n-- DIAGNOSTICS --\n");

    for (size_t i = 0; i < a->count; i++) {
        const Diagnostic *d = &a->items[i];

        printf(
            "id=%016llx time=%llu %s scope=%llu prev_scope=%llu\n",
            (unsigned long long)d->id.value,
            (unsigned long long)d->time,
            kind_str(d->kind),
            (unsigned long long)d->scope_id,
            (unsigned long long)d->prev_scope
        );

        if (d->anchor) {
            printf(
                " at node=%u line=%u col=%u\n",
                d->anchor->node_id,
                d->anchor->line,
                d->anchor->col
            );
        }
    }
}
