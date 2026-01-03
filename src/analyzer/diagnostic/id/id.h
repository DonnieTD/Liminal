#ifndef LIMINAL_DIAGNOSTIC_ID_H
#define LIMINAL_DIAGNOSTIC_ID_H

#include <stdint.h>
#include "../../constraint/constraint.h"


/*
 * DiagnosticId
 *
 * Stable semantic identity for a diagnostic.
 */
typedef struct DiagnosticId {
    uint64_t value;
} DiagnosticId;

/*
 * Derive a stable diagnostic identity from a constraint.
 */
DiagnosticId diagnostic_id_from_constraint(const Constraint *c);

/*
 * Render a diagnostic id (human-readable, stable).
 *
 * NOTE:
 *  - Formatting is centralized here
 *  - Consumers must not interpret DiagnosticId internals
 */
void diagnostic_id_render(DiagnosticId id);

#endif /* LIMINAL_DIAGNOSTIC_ID_H */
