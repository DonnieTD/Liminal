#ifndef LIMINAL_DIAGNOSTIC_STATS_H
#define LIMINAL_DIAGNOSTIC_STATS_H

#include <stdio.h>   /* FILE */
#include <stddef.h>  /* size_t */

#include "analyzer/analyzer.h"

typedef struct {
    size_t total;
    size_t by_kind[DIAG_KIND_MAX];
} DiagnosticStats;

/*
 * Compute statistics from diagnostics.
 * Does NOT allocate.
 */
void diagnostic_stats_compute(
    const DiagnosticArtifact *a,
    DiagnosticStats *out
);

/*
 * Emit statistics to a stream.
 * Does NOT allocate.
 */
void diagnostic_stats_emit(
    const DiagnosticStats *stats,
    FILE *out
);

#endif /* LIMINAL_DIAGNOSTIC_STATS_H */
