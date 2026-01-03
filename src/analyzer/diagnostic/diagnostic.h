#ifndef LIMINAL_ANALYZER_DIAGNOSTIC_H
#define LIMINAL_ANALYZER_DIAGNOSTIC_H

#include <stdint.h>
#include <stddef.h>
#include "../source_anchor.h"
#include "../diagnostic/id/id.h"
#include "../diagnostic/project/project.h"
#include "../diagnostic/serialize/serialize.h"
#include "../diagnostic/artifact/artifact.h"

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
    struct DiagnosticId id;
    DiagnosticKind kind;
    uint64_t time;
    uint64_t scope_id;
    uint64_t prev_scope;
    struct SourceAnchor *anchor;
} Diagnostic;


const char *diagnostic_kind_name(DiagnosticKind k);

void diagnostic_dump(const DiagnosticArtifact *a);

#endif /* LIMINAL_ANALYZER_DIAGNOSTIC_H */
