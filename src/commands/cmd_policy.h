#ifndef LIMINAL_CMD_POLICY_H
#define LIMINAL_CMD_POLICY_H

#include "policy/policy.h"
#include "analyzer/diagnostic.h"

/*
 * Print policy decision to stderr.
 * Returns non-zero on POLICY_DENY.
 */
int cmd_apply_policy(
    const Policy *policy,
    const DiagnosticArtifact *diagnostics
);

#endif /* LIMINAL_CMD_POLICY_H */
