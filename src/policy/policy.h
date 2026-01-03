#ifndef LIMINAL_POLICY_H
#define LIMINAL_POLICY_H

#include <stddef.h>
#include "../analyzer/analyzer.h"

typedef struct Policy {
    unsigned char deny_kind[DIAG_KIND_MAX];
    size_t        max_by_kind[DIAG_KIND_MAX];
    size_t        max_total;
} Policy;

typedef enum PolicyDecision {
    POLICY_ALLOW = 0,
    POLICY_WARN,
    POLICY_DENY
} PolicyDecision;

typedef struct PolicyRule {
    DiagnosticKind kind;
    size_t max_count;   /* 0 = unlimited */
    int deny;           /* boolean */
} PolicyRule;

Policy policy_default(void);

extern const struct Policy LIMINAL_DEFAULT_POLICY;

/*
 * Apply policy to diagnostics.
 *
 * Returns:
 *   0 → allowed
 *   non-zero → policy violation
 */
int cmd_apply_policy(
    const Policy *policy,
    const DiagnosticArtifact *diagnostics
);

PolicyDecision policy_evaluate(
    const Policy *policy,
    const DiagnosticArtifact *diagnostics
);


#endif /* LIMINAL_POLICY_H */
