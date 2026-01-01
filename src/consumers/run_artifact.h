#ifndef LIMINAL_RUN_ARTIFACT_H
#define LIMINAL_RUN_ARTIFACT_H

#include <stddef.h>
#include "analyzer/diagnostic.h"

/*
 * Loaded run snapshot.
 *
 * Immutable after load.
 */
typedef struct RunArtifact {
    char *run_id;
    char *input_path;
    unsigned long started_at;

    DiagnosticArtifact diagnostics;

    /* Timeline (opaque for now) */
    struct {
        unsigned long time;
        int step;
        unsigned int ast;
    } *timeline;

    size_t timeline_count;
} RunArtifact;

#endif /* LIMINAL_RUN_ARTIFACT_H */
