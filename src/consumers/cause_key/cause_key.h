#ifndef LIMINAL_CAUSE_KEY_H
#define LIMINAL_CAUSE_KEY_H

#include <stdint.h>
#include "../../executor/executor.h"
#include "../root/root.h"

typedef struct CauseKey {
    StepKind step;
    uint64_t ast_id;
    uint64_t scope_id;
} CauseKey;

/* MUST be exported — used by convergence_map */
int cause_key_equal(const CauseKey *a, const CauseKey *b);


int extract_cause_key(
    const RootChain *chain,
    CauseKey *out
);

#endif /* LIMINAL_CAUSE_KEY_H */
