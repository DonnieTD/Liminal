#ifndef LIMINAL_CONVERGENCE_BUILD_H
#define LIMINAL_CONVERGENCE_BUILD_H

#include "../map/map.h"
#include "../../root/root.h"
#include "../../../analyzer/analyzer.h"

/*
 * Build convergence groups from diagnostics + root chains.
 *
 * PURE:
 *  - no allocation outside ConvergenceMap
 *  - deterministic
 */
int build_convergence_map(
    const struct DiagnosticArtifact *diags,
    const struct RootChain *chains,
    ConvergenceMap *out
);




#endif /* LIMINAL_CONVERGENCE_BUILD_H */
