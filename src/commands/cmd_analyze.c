#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyzer/diagnostic.h"
#include "analyzer/diagnostic_serialize.h"

#include "consumers/diagnostic_render.h"
#include "consumers/diagnostic_stats.h"
#include "consumers/diagnostic_validate.h"

#define MAX_DIAGNOSTICS 256

static DiagnosticArtifact
load_diagnostics(const char *path)
{
    Diagnostic *buf = calloc(MAX_DIAGNOSTICS, sizeof(Diagnostic));
    size_t count = 0;

    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "error: failed to open %s\n", path);
        return (DiagnosticArtifact){0};
    }

    while (count < MAX_DIAGNOSTICS) {
        Diagnostic d = {0};
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
    snprintf(path, sizeof(path),
             "%s/diagnostics.ndjson", artifact_dir);

    DiagnosticArtifact a = load_diagnostics(path);
    if (!a.items)
        return 1;

    /* ---- VALIDATION GATE ---- */
    ValidationIssue issues[64];
    size_t issue_count =
        validate_diagnostics(&a, issues, 64);

    if (issue_count > 0) {
        printf("!! Diagnostic validation failed (%zu issues)\n",
               issue_count);
        free(a.items);
        return 1;
    }

    /* ---- STATS ---- */
    DiagnosticStats stats;
    diagnostic_stats_compute(&a, &stats);

    printf("\n== Diagnostic Stats ==\n");
    printf("total: %zu\n", stats.total);

    /* ---- RENDER ---- */
    printf("\n== Diagnostics ==\n");
    diagnostic_render_terminal(&a, stdout);

    free(a.items);
    return 0;
}
