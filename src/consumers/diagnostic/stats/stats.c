#include "./stats.h"
#include <string.h>

void diagnostic_stats_compute(
    const DiagnosticArtifact *a,
    DiagnosticStats *out
)
{
    if (!a || !out)
        return;

    memset(out, 0, sizeof(*out));
    out->total = a->count;

    for (size_t i = 0; i < a->count; i++) {
        DiagnosticKind k = a->items[i].kind;
        if ((size_t)k < sizeof(out->by_kind) / sizeof(out->by_kind[0])) {
            out->by_kind[k]++;
        }
    }
}
