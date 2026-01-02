#ifndef LIMINAL_ANALYZER_DIAGNOSTIC_H
#define LIMINAL_ANALYZER_DIAGNOSTIC_H

#include <stdint.h>
#include <stddef.h>
#include "analyzer/source_anchor.h"
#include "analyzer/diagnostic_id.h"

struct ASTNode;

typedef enum DiagnosticKind {
    DIAG_REDECLARATION = 0,
    DIAG_SHADOWING,
    DIAG_USE_BEFORE_DECLARE,
    DIAG_USE_AFTER_SCOPE_EXIT,

    /* Sentinel */
    DIAG_KIND_MAX
} DiagnosticKind;

typedef struct Diagnostic {
    DiagnosticId id;
    DiagnosticKind kind;
    uint64_t time;
    uint64_t scope_id;
    uint64_t prev_scope;
    struct SourceAnchor *anchor;
} Diagnostic;

typedef struct DiagnosticArtifact {
    Diagnostic *items;
    size_t count;
} DiagnosticArtifact;

struct World;

DiagnosticArtifact analyze_diagnostics(struct World *head);

void diagnostic_dump(const DiagnosticArtifact *a);

const char *diagnostic_kind_name(DiagnosticKind k);

#endif /* LIMINAL_ANALYZER_DIAGNOSTIC_H */
