#ifndef LIMINAL_ROOT_BUILD_H
#define LIMINAL_ROOT_BUILD_H

#include <stddef.h>
#include <stdint.h>
#include "../../../common/common.h"              // ✅ REQUIRED
#include "../../../executor/executor.h"
#include "./chain.h"

/*
 * Build a root-cause chain by walking the World timeline backwards.
 *
 * PURE:
 *  - no mutation
 *  - arena-only allocation
 */
RootChain build_root_chain(
    Arena *arena,
    const World *head,
    const Diagnostic *diag
);


#endif /* LIMINAL_ROOT_CHAIN_H */
