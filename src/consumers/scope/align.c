#include "./align.h"
#include "./signature.h"

size_t scope_align(
    const ScopeGraph *a,
    const ScopeGraph *b,
    ScopeAlignment *out,
    size_t cap
)
{
    size_t count = 0;

    /* Removed / unchanged */
    for (size_t i = 0; i < a->count && count < cap; i++) {
        uint64_t sig_a = scope_signature(&a->nodes[i]);
        int found = 0;

        for (size_t j = 0; j < b->count; j++) {
            if (sig_a == scope_signature(&b->nodes[j])) {
                found = 1;
                break;
            }
        }

        out[count++] = (ScopeAlignment){
            .old_sig = sig_a,
            .new_sig = found ? sig_a : 0,
            .kind = found ? SCOPE_UNCHANGED : SCOPE_REMOVED
        };
    }

    /* Added */
    for (size_t j = 0; j < b->count && count < cap; j++) {
        uint64_t sig_b = scope_signature(&b->nodes[j]);
        int found = 0;

        for (size_t i = 0; i < a->count; i++) {
            if (sig_b == scope_signature(&a->nodes[i])) {
                found = 1;
                break;
            }
        }

        if (!found) {
            out[count++] = (ScopeAlignment){
                .old_sig = 0,
                .new_sig = sig_b,
                .kind = SCOPE_ADDED
            };
        }
    }

    return count;
}
