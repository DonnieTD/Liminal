#ifndef LIMINAL_STEP_H
#define LIMINAL_STEP_H

#include <stdint.h>

/*
 * StepKind
 *
 * Enumerates the semantic reason *why* a World transition occurred.
 *
 * IMPORTANT:
 *  - StepKinds do NOT execute logic
 *  - They describe causality, not behavior
 *  - Analysis code relies on these being stable
 */
typedef enum StepKind {
    STEP_UNKNOWN = 0,

    /* Structural (lexical / control structure) */
    STEP_ENTER_SCOPE,
    STEP_EXIT_SCOPE,

    /* Control flow */
    STEP_CALL,
    STEP_RETURN,

    /* Declarations */
    STEP_DECLARE,   /* introduce a variable */

    /* Variable access (future) */
    STEP_USE,       /* read a variable */
    STEP_ASSIGN,    /* write a variable */

    /* Memory (future) */
    STEP_LOAD,
    STEP_STORE,

    /* Fallback / extension */
    STEP_OTHER
} StepKind;

/*
 * Step
 *
 * A Step represents the semantic *cause* of a World transition.
 *
 * Steps:
 *  - do NOT execute logic
 *  - do NOT own memory
 *  - are immutable once created
 *
 * Interpretation of `info` depends on StepKind:
 *  - ENTER / EXIT_SCOPE → scope id
 *  - DECLARE             → variable id
 *  - others              → kind-specific later
 */
typedef struct Step {
    StepKind kind;

    /* Pointer to AST node or frontend structure */
    void *origin;

    /* Kind-specific metadata (ids, line numbers, etc.) */
    uint64_t info;
} Step;

#endif /* LIMINAL_STEP_H */
