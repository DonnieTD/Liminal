#ifndef LIMINAL_ARTIFACT_EMIT_H
#define LIMINAL_ARTIFACT_EMIT_H

#include "analyzer/diagnostic.h"

struct World;

typedef struct {
    const char   *root;
    const char   *run_id;
    const char   *input_path;
    unsigned long started_at;

    const struct World *world_head;
} ArtifactContext;

void artifact_emit_all(
    const ArtifactContext *ctx,
    const DiagnosticArtifact *diagnostics
);

#endif
