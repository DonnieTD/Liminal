#include "analyzer/diagnostic.h"
#include "analyzer/constraint_engine.h"
#include "analyzer/constraint_diagnostic.h"
#include <stdlib.h>

DiagnosticArtifact analyze_diagnostics(struct World *head)
{
    Diagnostic *buf = calloc(256, sizeof(Diagnostic));
    size_t count = 0;

    /* --- Canonical semantic path --- */
    ConstraintArtifact constraints = analyze_constraints(head);
    count += constraint_to_diagnostic(
        &constraints,
        buf + count,
        256 - count
    );

    /* --- Temporary legacy path (shadowing only) --- */
    // count += analyze_shadowing(head, buf + count, 256 - count);

    return (DiagnosticArtifact){
        .items = buf,
        .count = count
    };
}
