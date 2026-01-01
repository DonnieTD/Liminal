#ifndef LIMINAL_TIMELINE_DIFF_H
#define LIMINAL_TIMELINE_DIFF_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/*
 * TimelineDiffKind
 *
 * Describes how a timeline step changed between two runs.
 */
typedef enum {
    TIMELINE_DIFF_UNCHANGED,
    TIMELINE_DIFF_ADDED,
    TIMELINE_DIFF_REMOVED,
    TIMELINE_DIFF_CHANGED
} TimelineDiffKind;

/*
 * TimelineStepView
 *
 * Minimal, normalized view of a timeline step.
 * Mirrors timeline.ndjson fields exactly.
 */
typedef struct {
    uint64_t time;
    uint32_t step_kind;
    uint32_t ast_id;
} TimelineStepView;

/*
 * TimelineDiff
 *
 * Represents a single divergence in semantic time.
 *
 * Invariants:
 *  - Deterministic
 *  - No allocation
 *  - One entry per divergent time index
 */
typedef struct {
    TimelineDiffKind kind;
    uint64_t time;

    /* Present for ADDED / REMOVED / CHANGED */
    TimelineStepView before;
    TimelineStepView after;
} TimelineDiff;

/*
 * timeline_diff
 *
 * Compare two normalized timeline streams.
 *
 * Rules:
 *  - Comparison is by `time` index
 *  - No sorting
 *  - No allocation
 *  - Deterministic output
 *
 * Returns:
 *  - Number of diff entries written to `out`
 */
size_t timeline_diff(
    const TimelineStepView *old_steps,
    size_t old_count,
    const TimelineStepView *new_steps,
    size_t new_count,
    TimelineDiff *out,
    size_t cap
);

/*
 * timeline_diff_first_line
 *
 * Temporary / bootstrap timeline diff.
 *
 * Returns:
 *  - (size_t)-1 if timelines are identical
 *  - first differing line index otherwise
 *
 * NOTE:
 *  - Deterministic
 *  - No allocation
 *  - Used to prove Stage 7 exit criteria
 */
size_t timeline_diff_first_line(
    FILE *a,
    FILE *b
);

#endif /* LIMINAL_TIMELINE_DIFF_H */
