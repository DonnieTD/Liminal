#include <stddef.h>

#include "consumers/convergence_map.h"
#include "consumers/root_chain.h"
#include "consumers/cause_key_extract.h"  
#include "consumers/cause_key.h"
#include "analyzer/diagnostic.h"

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
