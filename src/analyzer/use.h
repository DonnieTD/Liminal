#ifndef LIMINAL_ANALYZER_USE_H
#define LIMINAL_ANALYZER_USE_H

#include <stdint.h>
#include <stddef.h>

struct World;
struct ScopeLifetime;

/*
 * Classification of a variable use
 */
typedef enum UseKind {
    USE_OK = 0,
    USE_BEFORE_DECLARE,
    USE_AFTER_SCOPE
} UseKind;

/*
 * Result of analyzing a STEP_USE
 */
typedef struct UseReport {
    uint64_t time;
    uint64_t scope_id;
    uint64_t storage_id; /* UINT64_MAX if unresolved */
    UseKind kind;
} UseReport;

/*
 * Analyze STEP_USE events in a World timeline.
 *
 * - worlds: head of world list
 * - lifetimes: collected scope lifetimes
 * - lifetime_count: number of lifetimes
 * - out: output array
 * - cap: capacity of output array
 *
 * Returns number of reports written.
 */
size_t analyze_step_use(
    const struct World *worlds,
    const struct ScopeLifetime *lifetimes,
    size_t lifetime_count,
    UseReport *out,
    size_t cap
);

#endif /* LIMINAL_ANALYZER_USE_H */
