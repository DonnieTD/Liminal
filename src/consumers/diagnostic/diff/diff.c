#include "./diff.h"

/*
 * diagnostic_diff
 *
 * Compare two diagnostic artifacts by stable DiagnosticId.
 *
 * Output guarantees:
 *  - Each DiagnosticId appears at most once
 *  - No allocation
 *  - No sorting
 *  - Deterministic
 *
 * Complexity: O(n²) by design (small n, stable identity)
 */
size_t diagnostic_diff(
    const DiagnosticArtifact *old_run,
    const DiagnosticArtifact *new_run,
    DiagnosticDiff *out,
    size_t cap
)
{
    size_t count = 0;

    if (!out || cap == 0)
        return 0;

    /* ---- REMOVED / UNCHANGED ---- */
    for (size_t i = 0; old_run && i < old_run->count; i++) {
        DiagnosticId id = old_run->items[i].id;
        int found = 0;

        for (size_t j = 0; new_run && j < new_run->count; j++) {
            if (new_run->items[j].id.value == id.value) {
                found = 1;
                break;
            }
        }

        if (count >= cap)
            return count;

        out[count++] = (DiagnosticDiff){
            .kind = found ? DIFF_UNCHANGED : DIFF_REMOVED,
            .id   = id
        };
    }

    /* ---- ADDED ---- */
    for (size_t i = 0; new_run && i < new_run->count; i++) {
        DiagnosticId id = new_run->items[i].id;
        int found = 0;

        for (size_t j = 0; old_run && j < old_run->count; j++) {
            if (old_run->items[j].id.value == id.value) {
                found = 1;
                break;
            }
        }

        if (!found && count < cap) {
            out[count++] = (DiagnosticDiff){
                .kind = DIFF_ADDED,
                .id   = id
            };
        }
    }

    return count;
}
