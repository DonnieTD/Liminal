#ifndef LIMINAL_ROOT_CHAIN_H
#define LIMINAL_ROOT_CHAIN_H

#include <stddef.h>
#include <stdint.h>

#include "common/arena.h"              // ✅ REQUIRED
#include "executor/step.h"
#include "executor/world.h"
#include "analyzer/diagnostic.h"
#include "analyzer/diagnostic_id.h"

/*
 * RootRole
 *
 * Structural role a step plays in a diagnostic chain.
 */
typedef enum RootRole {
    ROOT_ROLE_CAUSE,
    ROOT_ROLE_AMPLIFIER,
    ROOT_ROLE_WITNESS,
    ROOT_ROLE_SUPPRESSOR
} RootRole;

typedef struct RootChainNode {
    uint64_t time;
    StepKind step;
    uint64_t ast_id;
    uint64_t scope_id;
    RootRole role;
} RootChainNode;

typedef struct RootChain {
    DiagnosticId diagnostic_id;
    RootChainNode *nodes;
    size_t count;
} RootChain;

/*
 * Build a root-cause chain by walking the World timeline backwards.
 *
 * PURE:
 *  - no mutation
 *  - arena-only allocation
 */
RootChain build_root_chain(
    Arena *arena,
    const World *head,
    const Diagnostic *diag
);

/* Render only */
void render_root_chain(const RootChain *chain);

#endif /* LIMINAL_ROOT_CHAIN_H */
