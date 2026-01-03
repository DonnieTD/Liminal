#include <stdio.h>
#include <inttypes.h>

#include "analyzer/analyzer.h"

/*
 * diagnostic_id_from_constraint
 *
 * Stable semantic identity for a diagnostic.
 */
DiagnosticId diagnostic_id_from_constraint(const Constraint *c)
{
    DiagnosticId id = {0};

    if (!c)
        return id;

    /* Simple structural hash — stable across runs */
    id.value ^= (uint64_t)c->kind;
    id.value ^= (uint64_t)c->time << 16;
    id.value ^= (uint64_t)c->scope_id << 32;

    return id;
}



void diagnostic_id_render(DiagnosticId id)
{
    printf("%016" PRIx64, id.value);
}
