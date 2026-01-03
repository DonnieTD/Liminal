#ifndef LIMINAL_ANALYZER_CONSTRAINT_H
#define LIMINAL_ANALYZER_CONSTRAINT_H

#include <stdint.h>
#include <stddef.h>

/* Stage 5.1 forward declaration */
struct SourceAnchor;
struct Diagnostic;
struct World; 

/*
 * ConstraintKind
 *
 * Enumerates semantic invariants derived from execution.
 *
 * Constraints are NOT diagnostics.
 * They express facts about semantic reality.
 */
typedef enum ConstraintKind {
    CONSTRAINT_USE_REQUIRES_DECLARATION,
    CONSTRAINT_REDECLARATION,
    CONSTRAINT_SHADOWING
} ConstraintKind;

/*
 * Constraint
 *
 * A single semantic invariant statement.
 */
typedef struct Constraint {
    ConstraintKind kind;
    uint64_t time;
    uint64_t scope_id;
    uint64_t storage_id;

    struct SourceAnchor *anchor;  /* may be NULL */
} Constraint;

/*
 * ConstraintArtifact
 *
 * Output of the constraint engine.
 */
typedef struct ConstraintArtifact {
    Constraint *items;
    size_t count;
} ConstraintArtifact;

ConstraintArtifact analyze_constraints(struct World *head);

size_t constraint_to_diagnostic(
    const ConstraintArtifact *constraints,
    struct Diagnostic *out,
    size_t cap
);

#endif /* LIMINAL_ANALYZER_CONSTRAINT_H */
