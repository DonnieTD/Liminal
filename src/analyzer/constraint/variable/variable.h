#ifndef LIMINAL_CONSTRAINT_VARIABLE_H
#define LIMINAL_CONSTRAINT_VARIABLE_H


struct World;
/*
 * Variable-related constraint extraction.
 *
 * Emits:
 *   - CONSTRAINT_REDECLARATION
 */
ConstraintArtifact analyze_variable_constraints(struct World *head);

#endif
