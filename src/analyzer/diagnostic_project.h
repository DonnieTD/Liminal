#ifndef LIMINAL_DIAGNOSTIC_PROJECT_H
#define LIMINAL_DIAGNOSTIC_PROJECT_H

#include "analyzer/diagnostic.h"
#include <stdio.h>

void diagnostic_project_ndjson(
    const DiagnosticArtifact *a,
    FILE *out
);

#endif
