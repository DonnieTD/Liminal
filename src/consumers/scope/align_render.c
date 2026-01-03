#include <stdio.h>
#include "./align.h"

static const char *scope_kind_str(ScopeChangeKind k)
{
    switch (k) {
    case SCOPE_ADDED:    return "ADDED";
    case SCOPE_REMOVED:  return "REMOVED";
    case SCOPE_MOVED:    return "MOVED";
    case SCOPE_SPLIT:    return "SPLIT";
    case SCOPE_MERGED:   return "MERGED";
    default:             return "UNCHANGED";
    }
}

void scope_align_render(
    const ScopeAlignment *a,
    size_t count,
    FILE *out
)
{
    fprintf(out,
        "%-18s %-18s %-10s\n",
        "OLD_SIG", "NEW_SIG", "CHANGE"
    );

    for (size_t i = 0; i < count; i++) {
        fprintf(
            out,
            "%016llx %016llx %-10s\n",
            (unsigned long long)a[i].old_sig,
            (unsigned long long)a[i].new_sig,
            scope_kind_str(a[i].kind)
        );
    }
}
