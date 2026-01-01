#ifndef LIMINAL_STEP_H
#define LIMINAL_STEP_H

#include <stdint.h>

/*
 * StepKind
 *
 * Enumerates semantic events in execution time.
 *
 * A Step answers:
 *   "What happened at this moment?"
 *
 * It does NOT encode:
 *   - control flow
 *   - data flow
 *   - ownership
 *
 * Those are derived later.
 */
typedef enum StepKind {
    STEP_UNKNOWN = 0,

    /* Program structure */
    STEP_ENTER_PROGRAM,
    STEP_EXIT_PROGRAM,
    STEP_ENTER_FUNCTION,
    STEP_EXIT_FUNCTION,

    /* Control flow */
    STEP_CALL,
    STEP_RETURN,

    /* Scopes */
    STEP_ENTER_SCOPE,
    STEP_EXIT_SCOPE,

    /* Variables */
    STEP_DECLARE,
    STEP_USE,
    STEP_ASSIGN,

    /* Memory (future) */
    STEP_LOAD,
    STEP_STORE,

    /* Catch-all */
    STEP_OTHER
} StepKind;

/*
 * Step
 *
 * A Step is a single semantic cause marker.
 *
 * Invariants:
 *  - Immutable once attached to a World
 *  - Owned by the Universe
 *  - Does NOT own memory
 *
 * Interpretation rules:
 *  - `origin` is opaque (usually ASTNode*)
 *  - `info` meaning depends on `kind`
 *
 * info semantics by kind:
 *  - STEP_ENTER_SCOPE / EXIT_SCOPE → scope_id
 *  - STEP_DECLARE / STEP_USE       → storage_id (or UINT64_MAX)
 *  - otherwise                     → unused (0)
 */
typedef struct Step {
    StepKind kind;
    void    *origin;
    uint64_t info;
} Step;

/*
 * Canonical stringification.
 *
 * Executor-owned.
 * Must stay stable for tooling.
 */
const char *step_kind_name(StepKind kind);

/*
 * Compatibility helper
 */
static inline const char *step_kind_str(StepKind kind)
{
    return step_kind_name(kind);
}

#endif /* LIMINAL_STEP_H */
