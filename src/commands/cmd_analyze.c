#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyzer/diagnostic.h"
#include "analyzer/diagnostic_serialize.h"

#include "consumers/diagnostic_render.h"
#include "consumers/diagnostic_stats.h"
#include "consumers/diagnostic_validate.h"

/*
 * Offline artifact analysis entry point
 *
 * Stage discipline:
 *  - consumes artifacts ONLY
 *  - no execution
 *  - no analysis
 *  - no allocation in consumers
 */

#define MAX_DIAGNOSTICS 256

static DiagnosticArtifact
load_diagnostics(const char *path)
{
    Diagnostic *buf = calloc(MAX_DIAGNOSTICS, sizeof(Diagnostic));
    size_t count = 0;

    if (!buf) {
        fprintf(stderr, "error: out of memory\n");
        return (DiagnosticArtifact){0};
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "error: failed to open %s\n", path);
        free(buf);
        return (DiagnosticArtifact){0};
    }

    while (count < MAX_DIAGNOSTICS) {
        Diagnostic d;
        memset(&d, 0, sizeof(d));

        if (!diagnostic_deserialize_line(f, &d))
            break;

        buf[count++] = d;
    }

    fclose(f);

    return (DiagnosticArtifact){
        .items = buf,
        .count = count
    };
}

int cmd_analyze(const char *artifact_dir)
{
    char path[512];

    snprintf(
        path,
        sizeof(path),
        "%s/diagnostics.ndjson",
        artifact_dir
    );

    DiagnosticArtifact a = load_diagnostics(path);
    if (!a.items)
        return 1;

    /* ------------------------------------------------------------
     * VALIDATION GATE
     * ------------------------------------------------------------ */

    ValidationIssue issues[64];
    size_t issue_count =
        validate_diagnostics(&a, issues, 64);

    if (issue_count > 0) {
        printf("!! Diagnostic validation failed (%zu issues)\n",
               issue_count);

        for (size_t i = 0; i < issue_count; i++) {
            printf(
                "  issue=%d id=%016llx\n",
                issues[i].kind,
                (unsigned long long)issues[i].id.value
            );
        }

        free(a.items);
        return 1;
    }

    /* ------------------------------------------------------------
     * STATS
     * ------------------------------------------------------------ */

    DiagnosticStats stats;
    diagnostic_stats_compute(&a, &stats);

    printf("\n== Diagnostic Stats ==\n");
    printf("total: %zu\n", stats.total);

    for (size_t i = 0; i < DIAG_KIND_MAX; i++) {
        if (stats.by_kind[i]) {
            printf(
                "  %s: %zu\n",
                diagnostic_kind_name((DiagnosticKind)i),
                stats.by_kind[i]
            );
        }
    }

    /* ------------------------------------------------------------
     * RENDER
     * ------------------------------------------------------------ */

    printf("\n== Diagnostics ==\n");
    diagnostic_render_terminal(&a, stdout);

    free(a.items);
    return 0;
}
