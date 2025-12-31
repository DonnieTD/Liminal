#pragma once
#include "analyzer/diagnostic.h"

typedef enum {
    VALIDATION_DUPLICATE_ID,
    VALIDATION_NON_MONOTONIC_TIME
} ValidationIssueKind;

typedef struct {
    ValidationIssueKind kind;
    DiagnosticId id;
} ValidationIssue;

size_t validate_diagnostics(
    const DiagnosticArtifact *a,
    ValidationIssue *out,
    size_t cap
);
