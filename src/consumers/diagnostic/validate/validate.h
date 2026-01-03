#ifndef LIMINAL_CONSUMERS_DIAGNOSTIC_VALIDATE_H
#define LIMINAL_CONSUMERS_DIAGNOSTIC_VALIDATE_H

#include "analyzer/analyzer.h"

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

#endif /* LIMINAL_CONSUMERS_DIAGNOSTIC_VALIDATE_H */
