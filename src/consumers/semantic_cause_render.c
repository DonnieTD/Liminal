#include <stdio.h>
#include "consumers/semantic_cause_diff.h"

static const char *cause_str(CauseChangeKind k)
{
    switch (k) {
    case CAUSE_NEW_STEP:     return "NEW_STEP";
    case CAUSE_STEP_REMOVED: return "REMOVED_STEP";
    case CAUSE_STEP_MOVED:   return "STEP_CHANGED";
    default:                 return "UNCHANGED";
    }
}

void semantic_cause_render(
    const SemanticCauseDiff *d,
    size_t count,
    FILE *out
)
{
    fprintf(out,
        "%-10s %-18s %-10s %-10s %-12s\n",
        "DIFF", "ID", "OLD_STEP", "NEW_STEP", "CAUSE"
    );

    for (size_t i = 0; i < count; i++) {
        fprintf(
            out,
            "%-10d %016llx %-10u %-10u %-12s\n",
            d[i].base.kind,
            (unsigned long long)d[i].base.id.value,
            d[i].old_step,
            d[i].new_step,
            cause_str(d[i].cause)
        );
    }
}
