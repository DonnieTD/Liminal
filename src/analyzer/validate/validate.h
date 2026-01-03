#ifndef LIMINAL_VALIDATE_H
#define LIMINAL_VALIDATE_H

#include <stddef.h>
#include <stdint.h>

/*
 * This file defines *structural validators* over the World timeline.
 *
 * Validators:
 *  - NEVER mutate Worlds
 *  - NEVER execute semantics
 *  - ONLY read derived structure from the trace
 *
 * Their job is to answer questions like:
 *   "Does this execution make sense?"
 *   "Is the structure well-formed?"
 *
 * This is where Liminal first learns to say:
 *   "This program is wrong."
 */

struct World;

/*
 * ScopeViolationKind
 *
 * Enumerates the kinds of structural errors we can detect
 * in scope behavior purely from the trace.
 *
 * These are NOT runtime errors.
 * These are *semantic shape violations*.
 */
typedef enum ScopeViolationKind {
    SCOPE_OK = 0,

    /*
     * An EXIT_SCOPE occurred when no scope was active.
     * This indicates:
     *  - frontend bug
     *  - executor bug
     *  - malformed input
     */
    SCOPE_EXIT_WITHOUT_ENTER,

    /*
     * A scope was entered but never exited.
     * This is detected at end-of-trace.
     */
    SCOPE_ENTER_WITHOUT_EXIT,

    /*
     * Scopes must be exited in LIFO order.
     * If scope A enters, then scope B enters,
     * scope B MUST exit before scope A.
     */
    SCOPE_NON_LIFO_EXIT,

    /*
     * The active_scope pointer stored in the World
     * does not match what the step-derived scope stack
     * says it *should* be.
     *
     * This detects:
     *  - executor bookkeeping bugs
     *  - illegal state transitions
     */
    SCOPE_ACTIVE_MISMATCH
} ScopeViolationKind;

/*
 * ScopeViolation
 *
 * A single detected violation of scope invariants.
 *
 * This is intentionally minimal:
 *  - what went wrong
 *  - when it happened
 *  - which scope was involved
 */
typedef struct ScopeViolation {
    ScopeViolationKind kind;

    /* World time at which violation was observed */
    uint64_t time;

    /* Scope id involved in the violation */
    uint64_t scope_id;
} ScopeViolation;

/*
 * validate_scope_invariants
 *
 * Walks the World timeline from beginning to end
 * and validates that scope ENTER / EXIT events form
 * a well-structured, properly nested tree.
 *
 * Inputs:
 *   - head : first World in timeline
 *   - out  : caller-provided array for violations
 *   - cap  : capacity of `out`
 *
 * Output:
 *   - returns number of violations written
 *
 * Notes:
 *   - Validation is PURE.
 *   - No allocation.
 *   - No mutation.
 *   - If cap is exceeded, results are truncated.
 */
size_t validate_scope_invariants(struct World *head,
                                 ScopeViolation *out,
                                 size_t cap);

#endif /* LIMINAL_VALIDATE_H */
