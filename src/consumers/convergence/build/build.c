#include <stddef.h>

#include "../map/map.h"
#include "../../root/root.h"
#include "../../cause_key/cause_key.h"
#include "../../../analyzer/analyzer.h"

/*
 * Build convergence groups across diagnostics by causal signature.
 */
int build_convergence_map(
    const DiagnosticArtifact *diags,
    const RootChain *chains,
    ConvergenceMap *out
)
{
    if (!diags || !chains || !out)
        return -1;

    for (size_t i = 0; i < diags->count; i++) {
        CauseKey key;

        if (extract_cause_key(&chains[i], &key) == 0) {
            convergence_map_add(out, &key, &diags->items[i]);
        }
    }

    return 0;
}
