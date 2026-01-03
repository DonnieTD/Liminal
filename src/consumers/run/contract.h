#ifndef LIMINAL_RUN_CONTRACT_H
#define LIMINAL_RUN_CONTRACT_H

/*
 * Artifact presence contract.
 *
 * Required:
 *   - meta.json
 *   - diagnostics.ndjson
 *
 * Optional:
 *   - timeline.ndjson
 */
typedef struct RunContract {
    int require_meta;
    int require_diagnostics;
    int allow_missing_timeline;
} RunContract;

/* Canonical Stage 7.1 contract */
static const RunContract LIMINAL_RUN_CONTRACT = {
    .require_meta        = 1,
    .require_diagnostics = 1,
    .allow_missing_timeline = 1
};

#endif /* LIMINAL_RUN_CONTRACT_H */
