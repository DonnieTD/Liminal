#include <stdio.h>
#include "./diff.h"

static const char *kind_str(SemanticDiffKind k)
{
    switch (k) {
    case SEMDIFF_ADDED:     return "ADDED";
    case SEMDIFF_REMOVED:   return "REMOVED";
    case SEMDIFF_MOVED:     return "MOVED";
    case SEMDIFF_UNCHANGED: return "UNCHANGED";
    default:                return "UNKNOWN";
    }
}

void semantic_diff_render(
    const SemanticDiff *diffs,
    size_t count,
    FILE *out
)
{
    fprintf(out,
        "%-10s %-18s %-10s %-10s\n",
        "KIND", "ID", "OLD", "NEW"
    );
    fprintf(out,
        "%-10s %-18s %-10s %-10s\n",
        "----------",
        "------------------",
        "----------",
        "----------"
    );

    for (size_t i = 0; i < count; i++) {
        fprintf(
            out,
            "%-10s %016llx %-10llu %-10llu\n",
            kind_str(diffs[i].kind),
            (unsigned long long)diffs[i].id.value,
            (unsigned long long)diffs[i].old_time,
            (unsigned long long)diffs[i].new_time
        );
    }
}
