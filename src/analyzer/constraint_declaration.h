//@header src/analyzer/constraint_declaration.h
#ifndef LIMINAL_CONSTRAINT_DECLARATION_H
#define LIMINAL_CONSTRAINT_DECLARATION_H

#include "analyzer/constraint.h"

struct World;

/*
 * Declaration-related constraint extraction.
 *
 * Emits:
 *   - CONSTRAINT_REDECLARATION
 *   - CONSTRAINT_SHADOWING
 */
ConstraintArtifact analyze_declaration_constraints(struct World *head);

#endif /* LIMINAL_CONSTRAINT_DECLARATION_H */
