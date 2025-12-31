#include "policy/policy.h"

PolicyDecision
policy_evaluate(
    const Policy *policy,
    const DiagnosticArtifact *diagnostics
)
{
    if (!policy || !diagnostics) {
        return POLICY_ALLOW;
    }

    size_t by_kind[DIAG_KIND_MAX] = {0};

    for (size_t i = 0; i < diagnostics->count; i++) {
        DiagnosticKind k = diagnostics->items[i].kind;

        /* Deny rule */
        if (policy->deny_kind[k]) {
            return POLICY_DENY;
        }

        by_kind[k]++;

        /* Per-kind cap */
        if (policy->max_by_kind[k] &&
            by_kind[k] > policy->max_by_kind[k]) {
            return POLICY_DENY;
        }
    }

    /* Total cap */
    if (policy->max_total &&
        diagnostics->count > policy->max_total) {
        return POLICY_DENY;
    }

    return POLICY_ALLOW;
}
