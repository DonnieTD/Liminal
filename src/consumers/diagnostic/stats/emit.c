#include <stdio.h>
#include "./stats.h"

void diagnostic_stats_emit(
    const DiagnosticStats *stats,
    FILE *out
)
{
    fprintf(out, "total: %zu\n", stats->total);

    for (size_t i = 0; i < DIAG_KIND_MAX; i++) {
        if (stats->by_kind[i]) {
            fprintf(out, "  kind[%zu]: %zu\n",
                    i, stats->by_kind[i]);
        }
    }
}
