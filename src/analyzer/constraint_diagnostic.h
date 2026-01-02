#ifndef LIMINAL_ANALYZER_CONSTRAINT_DIAGNOSTIC_H
#define LIMINAL_ANALYZER_CONSTRAINT_DIAGNOSTIC_H

#include <stddef.h>
#include "analyzer/constraint.h"
#include "analyzer/diagnostic.h"

/*
 * Project constraints into diagnostics.
 *
 * Returns number of diagnostics written.
 */
size_t constraint_to_diagnostic(
    const ConstraintArtifact *constraints,
    Diagnostic *out,
    size_t cap
);

#endif /* LIMINAL_ANALYZER_CONSTRAINT_DIAGNOSTIC_H */
