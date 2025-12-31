#include "consumers/diagnostic_validate.h"

/*
 * validate_diagnostics
 *
 * Structural gate over DiagnosticArtifact.
 *
 * Detects:
 *   - duplicate DiagnosticId
 *   - non-monotonic time
 *
 * Discipline:
 *   - no allocation
 *   - no mutation
 *   - deterministic
 */
size_t validate_diagnostics(
    const DiagnosticArtifact *a,
    ValidationIssue *out,
    size_t cap
)
{
    size_t count = 0;

    if (!a || !out || cap == 0)
        return 0;

    /* ---- Duplicate IDs ---- */
    for (size_t i = 0; i < a->count; i++) {
        for (size_t j = i + 1; j < a->count; j++) {
            if (a->items[i].id.value == a->items[j].id.value) {
                if (count < cap) {
                    out[count++] = (ValidationIssue){
                        .kind = VALIDATION_DUPLICATE_ID,
                        .id   = a->items[i].id
                    };
                }
            }
        }
    }

    /* ---- Monotonic time ---- */
    for (size_t i = 1; i < a->count; i++) {
        if (a->items[i].time < a->items[i - 1].time) {
            if (count < cap) {
                out[count++] = (ValidationIssue){
                    .kind = VALIDATION_NON_MONOTONIC_TIME,
                    .id   = a->items[i].id
                };
            }
        }
    }

    return count;
}
