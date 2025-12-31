#include <stdio.h>
#include "commands/cmd_policy.h"
#include "policy/policy.h"

int
cmd_apply_policy(
    const Policy *policy,
    const DiagnosticArtifact *diagnostics
)
{
    PolicyDecision d = policy_evaluate(policy, diagnostics);

    switch (d) {
    case POLICY_ALLOW:
        return 0;

    case POLICY_WARN:
        fprintf(stderr, "policy warning\n");
        return 0;

    case POLICY_DENY:
        fprintf(stderr, "policy denied execution\n");
        return 1;
    }

    return 0;
}
