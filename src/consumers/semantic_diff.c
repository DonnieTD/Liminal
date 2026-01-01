#include "consumers/semantic_diff.h"

/*
 * semantic_diff
 *
 * Identity axis: DiagnosticId
 * Temporal axis: time
 */
size_t semantic_diff(
    const DiagnosticArtifact *old_run,
    const DiagnosticArtifact *new_run,
    SemanticDiff *out,
    size_t cap
)
{
    size_t count = 0;

    if (!out || cap == 0)
        return 0;

    /* ---- REMOVED / UNCHANGED / MOVED ---- */
    for (size_t i = 0; old_run && i < old_run->count; i++) {
        const Diagnostic *d_old = &old_run->items[i];
        int found = 0;

        for (size_t j = 0; new_run && j < new_run->count; j++) {
            const Diagnostic *d_new = &new_run->items[j];
            if (d_new->id.value == d_old->id.value) {
                found = 1;
                if (count >= cap)
                    return count;

                if (d_new->time == d_old->time) {
                    out[count++] = (SemanticDiff){
                        .kind = SEMDIFF_UNCHANGED,
                        .id = d_old->id,
                        .old_time = d_old->time,
                        .new_time = d_new->time
                    };
                } else {
                    out[count++] = (SemanticDiff){
                        .kind = SEMDIFF_MOVED,
                        .id = d_old->id,
                        .old_time = d_old->time,
                        .new_time = d_new->time
                    };
                }
                break;
            }
        }

        if (!found && count < cap) {
            out[count++] = (SemanticDiff){
                .kind = SEMDIFF_REMOVED,
                .id = d_old->id,
                .old_time = d_old->time,
                .new_time = 0
            };
        }
    }

    /* ---- ADDED ---- */
    for (size_t i = 0; new_run && i < new_run->count; i++) {
        const Diagnostic *d_new = &new_run->items[i];
        int found = 0;

        for (size_t j = 0; old_run && j < old_run->count; j++) {
            if (old_run->items[j].id.value == d_new->id.value) {
                found = 1;
                break;
            }
        }

        if (!found && count < cap) {
            out[count++] = (SemanticDiff){
                .kind = SEMDIFF_ADDED,
                .id = d_new->id,
                .old_time = 0,
                .new_time = d_new->time
            };
        }
    }

    return count;
}
