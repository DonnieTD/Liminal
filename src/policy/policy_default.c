#include "policy/policy.h"
#include <string.h>

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
