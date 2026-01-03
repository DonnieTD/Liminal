#include "common/common.h"
#include "consumers/consumers.h"
#include "analyzer/analyzer.h"
#include "executor/executor.h"
#include "consumers/root/chain/build.h"
#include "consumers/root/chain/chain.h"



void build_and_render_root_chains(
    const World *world,
    const DiagnosticArtifact *diags
)
{
    if (diags->count == 0)
        return;

    Arena arena;
    arena_init(&arena, 4096);

    for (size_t i = 0; i < diags->count; i++) {
        struct RootChain chain = build_root_chain(
            &arena,
            world,
            &diags->items[i]
        );
        render_root_chain(&chain);
    }

    arena_destroy(&arena);
}
