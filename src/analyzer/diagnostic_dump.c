#include "analyzer/diagnostic.h"
#include <stdio.h>

static const char *kind_str(DiagnosticKind k)
{
    switch (k) {
    case DIAG_REDECLARATION: return "REDECLARATION";
    case DIAG_SHADOWING: return "SHADOWING";
    case DIAG_USE_BEFORE_DECLARE: return "USE_BEFORE_DECLARE";
    case DIAG_USE_AFTER_SCOPE_EXIT: return "USE_AFTER_SCOPE_EXIT";
    default: return "UNKNOWN";
    }
}

void diagnostic_dump(const DiagnosticArtifact *a)
{
    if (a->count == 0)
        return;

    printf("\n-- DIAGNOSTICS --\n");

    for (size_t i = 0; i < a->count; i++) {
        const Diagnostic *d = &a->items[i];
        printf(
            "time=%llu %s name=%s scope=%llu prev_scope=%llu\n",
            (unsigned long long)d->time,
            kind_str(d->kind),
            d->name ? d->name : "?",
            (unsigned long long)d->scope_id,
            (unsigned long long)d->previous_scope_id
        );
    }
}
