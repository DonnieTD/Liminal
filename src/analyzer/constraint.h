#pragma once
#include <stdint.h>
#include <stddef.h>

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
