#ifndef LIMINAL_CAUSE_KEY_EXTRACT_H
#define LIMINAL_CAUSE_KEY_EXTRACT_H

#include "consumers/root_chain.h"
#include "consumers/cause_key.h"

/*
 * Extract a stable semantic cause key from a root chain.
 *
 * Returns:
 *   0 on success
 *  -1 if no valid cause key can be derived
 *
 * PURE:
 *  - no allocation
 *  - no mutation
 */
int extract_cause_key(
    const RootChain *chain,
    CauseKey *out
);

#endif /* LIMINAL_CAUSE_KEY_EXTRACT_H */
