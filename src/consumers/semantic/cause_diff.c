#include "./cause_diff.h"
#include "../diagnostic/diagnostic.h"

CauseChangeKind classify_cause(
    const DiagnosticAnchor *a,
    const DiagnosticAnchor *b,
    uint32_t *old_step,
    uint32_t *new_step
)
{
    if (!a && b) {
        *old_step = 0;
        *new_step = b->cause.step_kind;
        return CAUSE_NEW_STEP;
    }

    if (a && !b) {
        *old_step = a->cause.step_kind;
        *new_step = 0;
        return CAUSE_STEP_REMOVED;
    }

    if (a && b && a->cause.step_kind != b->cause.step_kind) {
        *old_step = a->cause.step_kind;
        *new_step = b->cause.step_kind;
        return CAUSE_STEP_MOVED;
    }

    return CAUSE_NONE;
}
