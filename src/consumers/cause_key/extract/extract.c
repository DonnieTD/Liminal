#include "consumers/consumers.h"
#include "executor/executor.h"

/*
 * Extract a CauseKey from a RootChain.
 *
 * Stage 7 discipline:
 * - deterministic
 * - no allocation
 * - no mutation
 *
 * Strategy:
 * - Use the FIRST node in the chain (closest causal event)
 */
int extract_cause_key(
    const RootChain *chain,
    CauseKey *out
)
{
    if (!chain || !out || chain->count == 0) {
        return 1;
    }

    const RootChainNode *n = &chain->nodes[0];

    out->step     = n->step;
    out->ast_id   = n->ast_id;
    out->scope_id = n->scope_id;

    return 0;
}
