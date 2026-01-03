#include "policy/policy.h"
#include <string.h>

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




Policy policy_default(void)
{
    Policy p;
    memset(&p, 0, sizeof(p));

    /* hard errors */
    p.deny_kind[DIAG_USE_BEFORE_DECLARE] = 1;
    p.deny_kind[DIAG_REDECLARATION]      = 1;

    /* warnings allowed but capped */
    p.max_by_kind[DIAG_SHADOWING] = 16;

    /* global budget */
    p.max_total = 64;

    return p;
}


// static const PolicyRule DEFAULT_RULES[] = {
//     /* Hard denies */
//     { DIAG_USE_BEFORE_DECLARE, 0, 1 },
//     { DIAG_REDECLARATION,      0, 1 },

//     /* Soft limits */
//     { DIAG_SHADOWING,         16, 0 }
// };

const Policy LIMINAL_DEFAULT_POLICY = {
    .deny_kind = {
        [DIAG_USE_BEFORE_DECLARE] = 1,
        [DIAG_REDECLARATION]      = 1
    },
    .max_by_kind = {
        [DIAG_SHADOWING] = 16
    },
    .max_total = 64
};

