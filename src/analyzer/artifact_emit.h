#ifndef LIMINAL_ARTIFACT_EMIT_H
#define LIMINAL_ARTIFACT_EMIT_H

#include "analyzer/diagnostic.h"
#include "stdbool.h"

typedef struct {
    const char *root;
    const char *run_id;
    const char *input_path;
    unsigned long started_at;
} ArtifactContext;

bool artifact_emit_all(
    const ArtifactContext *ctx,
    const DiagnosticArtifact *diagnostics
);

#endif
