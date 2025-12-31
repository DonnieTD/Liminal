#include "analyzer/diagnostic.h"
#include "analyzer/shadow.h"
#include "analyzer/use_validate.h"

#include <stdlib.h>

DiagnosticArtifact analyze_diagnostics(struct World *head)
{
    size_t cap = 128;
    Diagnostic *buf = calloc(cap, sizeof(Diagnostic));
    size_t count = 0;

    count += analyze_shadowing(head, buf + count, cap - count);
    count += analyze_use_validation(head, buf + count, cap - count);

    return (DiagnosticArtifact){
        .items = buf,
        .count = count
    };
}
