#ifndef LIMINAL_CONSTRAINT_VARIABLE_H
#define LIMINAL_CONSTRAINT_VARIABLE_H

#include "analyzer/constraint.h"
#include "executor/world.h"

/*
 * Variable-related constraint extraction.
 *
 * Emits:
 *   - CONSTRAINT_REDECLARATION
 */
ConstraintArtifact analyze_variable_constraints(struct World *head);

#endif
