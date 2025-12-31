#pragma once

#include <stdio.h>

struct DiagnosticArtifact;

void diagnostic_render_terminal(
    const struct DiagnosticArtifact *a,
    FILE *out
);
