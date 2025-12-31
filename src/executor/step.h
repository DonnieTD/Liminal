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

    /* Structural */
    STEP_ENTER_PROGRAM,
    STEP_EXIT_PROGRAM,

    STEP_ENTER_FUNCTION,
    STEP_EXIT_FUNCTION,


    /* Statements */
    STEP_RETURN,

    /* Existing (unused yet) */
    STEP_ENTER_SCOPE,
    STEP_EXIT_SCOPE,
    STEP_CALL,
    STEP_DECLARE,
    STEP_USE,
    STEP_ASSIGN,
    STEP_LOAD,
    STEP_STORE,
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
    void    *origin;   // <-- opaque pointer
    uint64_t info;     // <-- kind-specific ID
} Step;


#endif /* LIMINAL_STEP_H */
