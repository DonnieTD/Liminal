#include "analyzer/diagnostic_project.h"
#include "analyzer/diagnostic.h"

void diagnostic_project_ndjson(
    const DiagnosticArtifact *a,
    FILE *out
)
{
    for (size_t i = 0; i < a->count; i++) {
        const Diagnostic *d = &a->items[i];

        fprintf(
            out,
            "{"
            "\"id\":\"%016llx\","
            "\"time\":%llu,"
            "\"kind\":\"%s\","
            "\"scope\":%llu,"
            "\"prev_scope\":%llu",
            (unsigned long long)d->id.value,
            (unsigned long long)d->time,
            diagnostic_kind_name(d->kind),
            (unsigned long long)d->scope_id,
            (unsigned long long)d->prev_scope
        );

        if (d->anchor) {
            fprintf(
                out,
                ",\"anchor\":{"
                "\"node\":%u,"
                "\"line\":%u,"
                "\"col\":%u"
                "}",
                d->anchor->node_id,
                d->anchor->line,
                d->anchor->col
            );
        }

        fprintf(out, "}\n");
    }
}
