#include "policy/policy.h"

PolicyResult policy_evaluate(
    const Policy *p,
    const DiagnosticArtifact *a,
    const DiagnosticStats *s
)
{
    (void)a; /* reserved for future policy types */

    /* ---- TOTAL BUDGET ---- */
    if (p->max_total && s->total > p->max_total) {
        return POLICY_FAIL;
    }

    /* ---- PER-KIND BUDGETS ---- */
    for (size_t i = 0; i <= DIAG_USE_AFTER_SCOPE_EXIT; i++) {
        if (p->max_by_kind[i] &&
            s->by_kind[i] > p->max_by_kind[i]) {
            return POLICY_FAIL;
        }
    }

    /* ---- DENY LIST ---- */
    for (size_t i = 0; i <= DIAG_USE_AFTER_SCOPE_EXIT; i++) {
        if (p->deny_kind[i] && s->by_kind[i] > 0) {
            return POLICY_FAIL;
        }
    }

    return POLICY_PASS;
}
