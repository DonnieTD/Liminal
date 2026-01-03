#ifndef LIMINAL_ROOT_CHAIN_H
#define LIMINAL_ROOT_CHAIN_H

#include <stddef.h>
#include <stdint.h>

#include "../../../common/common.h"              // ✅ REQUIRED
#include "../../../executor/executor.h"
#include "../../../analyzer/analyzer.h"


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


/* Render only */
void render_root_chain(const RootChain *chain);

#endif /* LIMINAL_ROOT_CHAIN_H */
