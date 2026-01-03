#ifndef LIMINAL_SEMANTIC_DIFF_H
#define LIMINAL_SEMANTIC_DIFF_H

#include <stddef.h>
#include "../../analyzer/analyzer.h"

typedef enum SemanticDiffKind {
    SEMDIFF_ADDED,
    SEMDIFF_REMOVED,
    SEMDIFF_UNCHANGED,
    SEMDIFF_MOVED   /* same id, different time */
} SemanticDiffKind;

typedef struct SemanticDiff {
    SemanticDiffKind kind;
    DiagnosticId id;
    uint64_t old_time;
    uint64_t new_time;
} SemanticDiff;

/*
 * Compute semantic diff between two runs.
 *
 * No allocation.
 * Deterministic.
 * Stable ordering.
 */
size_t semantic_diff(
    const DiagnosticArtifact *old_run,
    const DiagnosticArtifact *new_run,
    SemanticDiff *out,
    size_t cap
);

#endif /* LIMINAL_SEMANTIC_DIFF_H */
