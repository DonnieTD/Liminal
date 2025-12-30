#ifndef LIMINAL_STEP_H
#define LIMINAL_STEP_H

#include <stdint.h>

/*
 * Step
 *
 * A Step represents the semantic cause of a World transition.
 *
 * Typically corresponds to:
 * - an AST node
 * - a statement
 * - an expression evaluation
 *
 * Steps do not execute logic.
 * They exist only to explain *why* a World exists.
 */

typedef enum StepKind {
    STEP_UNKNOWN = 0,

    /* Structural */
    STEP_ENTER_SCOPE,
    STEP_EXIT_SCOPE,

    /* Control */
    STEP_CALL,
    STEP_RETURN,

    /* Evaluation */
    STEP_ASSIGN,
    STEP_LOAD,
    STEP_STORE,

    /* Placeholder for future extension */
    STEP_OTHER
} StepKind;

typedef struct Step {
    /* Kind of semantic action */
    StepKind kind;

    /* Opaque pointer to AST node or frontend structure */
    void *origin;

    /* Optional metadata (line number, source span, etc.) */
    uint64_t info;
} Step;

#endif /* LIMINAL_STEP_H */
