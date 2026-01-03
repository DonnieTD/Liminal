#ifndef LIMINAL_CMD_POLICY_H
#define LIMINAL_CMD_POLICY_H

#include "../../policy/policy.h"

struct DiagnosticArtifact;
/*
 * Print policy decision to stderr.
 * Returns non-zero on POLICY_DENY.
 */
int cmd_apply_policy(
    const Policy *policy,
    const struct DiagnosticArtifact *diagnostics
);

#endif /* LIMINAL_CMD_POLICY_H */
