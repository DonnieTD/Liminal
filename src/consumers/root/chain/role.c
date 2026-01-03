#include "consumers/consumers.h"
#include "analyzer/analyzer.h"

/*
 * Assign semantic roles based on diagnostic kind and causal distance.
 *
 * nodes[0] is the closest cause to the diagnostic.
 */
void assign_root_chain_roles(RootChain *chain, DiagnosticKind kind)
{
    if (!chain || chain->count == 0)
        return;

    for (size_t i = 0; i < chain->count; i++) {
        RootChainNode *n = &chain->nodes[i];

        if (i == 0) {
            /* Closest event to diagnostic */
            n->role = ROOT_ROLE_CAUSE;
            continue;
        }

        switch (kind) {
            case DIAG_USE_BEFORE_DECLARE:
            case DIAG_REDECLARATION:
            case DIAG_SHADOWING:
                if (n->step == STEP_DECLARE ||
                    n->step == STEP_USE) {
                    n->role = ROOT_ROLE_AMPLIFIER;
                } else {
                    n->role = ROOT_ROLE_WITNESS;
                }
                break;

            default:
                n->role = ROOT_ROLE_WITNESS;
                break;
        }
    }
}
