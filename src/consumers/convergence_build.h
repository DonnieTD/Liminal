#ifndef LIMINAL_CONVERGENCE_BUILD_H
#define LIMINAL_CONVERGENCE_BUILD_H

#include "analyzer/diagnostic.h"
#include "consumers/root_chain.h"
#include "consumers/convergence_map.h"

/*
 * Build convergence groups from diagnostics + root chains.
 *
 * PURE:
 *  - no allocation outside ConvergenceMap
 *  - deterministic
 */
int build_convergence_map(
    const DiagnosticArtifact *diags,
    const RootChain *chains,
    ConvergenceMap *out
);

#endif /* LIMINAL_CONVERGENCE_BUILD_H */
