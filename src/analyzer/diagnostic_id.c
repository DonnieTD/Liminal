#include "analyzer/diagnostic_id.h"

/*
 * Stable identity derivation.
 *
 * This is NOT a hash.
 * This is semantic composition.
 */
DiagnosticId diagnostic_id_from_constraint(const Constraint *c)
{
    DiagnosticId id = {0};

    if (!c)
        return id;

    /*
     * Bit layout (documented, stable):
     *
     * [ 16 bits kind ][ 16 bits scope ][ 32 bits time ]
     *
     * storage_id intentionally excluded for now:
     *   - unstable across refactors
     *   - can be added later if needed
     */
    id.value =
        ((uint64_t)c->kind     << 48) |
        ((uint64_t)c->scope_id << 32) |
        (uint64_t)c->time;

    return id;
}
