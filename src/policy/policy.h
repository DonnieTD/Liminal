#pragma once

#include <stddef.h>
#include "analyzer/diagnostic.h"
#include "consumers/diagnostic_stats.h"

typedef enum {
    POLICY_PASS = 0,
    POLICY_FAIL = 1
} PolicyResult;

/*
 * Policy configuration.
 * Zero means "no limit".
 */
typedef struct {
    size_t max_total;

    size_t max_by_kind[DIAG_USE_AFTER_SCOPE_EXIT + 1];

    /* hard fail kinds */
    unsigned char deny_kind[DIAG_USE_AFTER_SCOPE_EXIT + 1];
} Policy;

/*
 * Evaluate policy against diagnostics + stats.
 *
 * No allocation.
 * No IO.
 */
PolicyResult policy_evaluate(
    const Policy *policy,
    const DiagnosticArtifact *diagnostics,
    const DiagnosticStats *stats
);
