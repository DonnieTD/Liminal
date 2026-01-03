#ifndef LIMINAL_CAUSE_KEY_EXTRACT_H
#define LIMINAL_CAUSE_KEY_EXTRACT_H

#include "../cause_key.h"
#include "../../root/root.h"

int extract_cause_key(
    const RootChain *chain,
    CauseKey *out
);

#endif /* LIMINAL_CAUSE_KEY_EXTRACT_H */
