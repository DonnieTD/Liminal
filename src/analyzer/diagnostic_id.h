#ifndef LIMINAL_DIAGNOSTIC_ID_H
#define LIMINAL_DIAGNOSTIC_ID_H

#include <stdint.h>
#include "analyzer/constraint.h"

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
 *
 * PURE:
 *  - no allocation
 *  - no globals
 *  - deterministic
 */
DiagnosticId diagnostic_id_from_constraint(const Constraint *c);

#endif /* LIMINAL_DIAGNOSTIC_ID_H */
