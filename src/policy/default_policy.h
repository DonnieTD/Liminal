#ifndef LIMINAL_DEFAULT_POLICY_H
#define LIMINAL_DEFAULT_POLICY_H

#include "policy/policy.h"

/*
 * Default Liminal policy.
 *
 * Philosophy:
 *  - Structural violations deny
 *  - Shadowing warns
 *  - Redeclaration denies
 *  - Use-before-declare denies
 */
extern const Policy LIMINAL_DEFAULT_POLICY;

#endif /* LIMINAL_DEFAULT_POLICY_H */
