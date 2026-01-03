#ifndef LIMINAL_ARTIFACT_EMIT_H
#define LIMINAL_ARTIFACT_EMIT_H


struct World;
struct Diagnostic;

typedef struct DiagnosticArtifact {
    struct Diagnostic *items;
    size_t count;
} DiagnosticArtifact;

typedef struct {
    const char   *root;
    const char   *run_id;
    const char   *input_path;
    unsigned long started_at;

    const struct World *world_head;
} ArtifactContext;

DiagnosticArtifact analyze_diagnostics(struct World *head);


void artifact_emit_all(
    const ArtifactContext *ctx,
    const DiagnosticArtifact *diagnostics
);

#endif
