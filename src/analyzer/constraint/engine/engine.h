#ifndef LIMINAL_CONSTRAINT_ENGINE_H
#define LIMINAL_CONSTRAINT_ENGINE_H

#include "analyzer/constraint/constraint.h"

/*
 * Constraint engine entry point.
 *
 * Consumes a World timeline and produces semantic constraints.
 */
ConstraintArtifact analyze_constraints(struct World *head);

#endif /* LIMINAL_CONSTRAINT_ENGINE_H */
