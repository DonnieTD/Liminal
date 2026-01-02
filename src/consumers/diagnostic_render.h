#ifndef LIMINAL_CONSUMERS_DIAGNOSTIC_RENDER_H
#define LIMINAL_CONSUMERS_DIAGNOSTIC_RENDER_H

#include <stdio.h>

struct DiagnosticArtifact;

void diagnostic_render_terminal(
    const struct DiagnosticArtifact *a,
    FILE *out
);

#endif /* LIMINAL_CONSUMERS_DIAGNOSTIC_RENDER_H */
