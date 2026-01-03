#ifndef LIMINAL_ROOT_CHAIN_ROLE_H
#define LIMINAL_ROOT_CHAIN_ROLE_H

#include "./chain.h"
#include "analyzer/analyzer.h"

/*
 * Assign semantic roles to nodes in a root-cause chain.
 *
 * nodes[0] is the closest causal event to the diagnostic.
 */
void assign_root_chain_roles(
    RootChain *chain,
    DiagnosticKind kind
);

#endif /* LIMINAL_ROOT_CHAIN_ROLE_H */
