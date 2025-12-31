#include "policy/default_policy.h"
#include "policy/policy.h"

static const PolicyRule DEFAULT_RULES[] = {
    /* Hard denies */
    { DIAG_USE_BEFORE_DECLARE, 0, 1 },
    { DIAG_REDECLARATION,      0, 1 },

    /* Soft limits */
    { DIAG_SHADOWING,         16, 0 }
};

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
