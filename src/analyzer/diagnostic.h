#pragma once
#include <stdint.h>
#include <stddef.h>

struct ASTNode;

typedef enum DiagnosticKind {
    DIAG_REDECLARATION,
    DIAG_SHADOWING,
    DIAG_USE_BEFORE_DECLARE,
    DIAG_USE_AFTER_SCOPE_EXIT,
} DiagnosticKind;

typedef struct Diagnostic {
    DiagnosticKind kind;

    uint64_t time;

    uint64_t scope_id;
    uint64_t previous_scope_id;

    const char *name;

    const struct ASTNode *origin;
    const struct ASTNode *previous_origin;
} Diagnostic;

typedef struct DiagnosticArtifact {
    Diagnostic *items;
    size_t count;
} DiagnosticArtifact;

struct World;

DiagnosticArtifact analyze_diagnostics(struct World *head);
void diagnostic_dump(const DiagnosticArtifact *a);


