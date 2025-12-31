#pragma once
#include "analyzer/diagnostic.h"

typedef enum {
    DIFF_ADDED,
    DIFF_REMOVED,
    DIFF_UNCHANGED
} DiagnosticDiffKind;

typedef struct {
    DiagnosticDiffKind kind;
    DiagnosticId id;
} DiagnosticDiff;

size_t diagnostic_diff(
    const DiagnosticArtifact *old_run,
    const DiagnosticArtifact *new_run,
    DiagnosticDiff *out,
    size_t cap
);
