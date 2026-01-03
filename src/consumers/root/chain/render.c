#include <stdio.h>

#include "./chain.h"
#include "executor/executor.h"
#include "analyzer/analyzer.h"

static const char *role_str(RootRole r)
{
    switch (r) {
        case ROOT_ROLE_CAUSE:      return "CAUSE";
        case ROOT_ROLE_AMPLIFIER: return "AMPLIFIER";
        case ROOT_ROLE_WITNESS:   return "WITNESS";
        case ROOT_ROLE_SUPPRESSOR:return "SUPPRESSOR";
        default:                  return "?";
    }
}

void render_root_chain(const RootChain *chain)
{
    printf("\nROOT CAUSE CHAIN (diag=");
    diagnostic_id_render(chain->diagnostic_id);
    printf(")\n");
    printf("----------------------------\n");

    for (size_t i = 0; i < chain->count; i++) {
        const RootChainNode *n = &chain->nodes[i];
        printf(
            "[t=%llu] %-11s step=%s ast=%llu scope=%llu\n",
            (unsigned long long)n->time,
            role_str(n->role),
            step_kind_str(n->step),
            (unsigned long long)n->ast_id,
            (unsigned long long)n->scope_id
        );
    }
}
