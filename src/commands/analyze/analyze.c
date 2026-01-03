#include <stdio.h>
#include <stdlib.h>

#include "./analyze.h"
#include "common/common.h"
#include "analyzer/analyzer.h"
#include "consumers/consumers.h"
#include "consumers/root/chain/build.h"
#include "consumers/root/chain/role.h"
#include "consumers/convergence/build/build.h"
#include "consumers/convergence/render/render.h"
#include "consumers/fix_surface/fix_surface_render.h"


/*
 * cmd_analyze
 *
 * Stage 7:
 *  - derive diagnostics
 *  - derive root chains (ephemeral)
 *  - derive convergence
 *  - derive minimal fix surface
 *
 * NO mutation
 * NO persistence
 * NO cross-stage storage
 */
int cmd_analyze(const struct World *world)
{
    if (!world) {
        fprintf(stderr, "analyze: no world provided\n");
        return 1;
    }

    /* --- Diagnostics --- */

    struct DiagnosticArtifact diags =
        analyze_diagnostics((struct World *)world);

    if (diags.count == 0) {
        printf("No diagnostics.\n");
        return 0;
    }

    /* --- Arena for derived artifacts --- */

    Arena arena;
    arena_init(&arena, 32 * 1024);

    /* --- Root chains --- */

    struct RootChain *chains =
        arena_alloc(&arena, diags.count * sizeof(struct RootChain));

    for (size_t i = 0; i < diags.count; i++) {
        chains[i] = build_root_chain(
            &arena,
            world,
            &diags.items[i]
        );

        assign_root_chain_roles(
            &chains[i],
            diags.items[i].kind
        );
    }

    /* --- Convergence --- */

    ConvergenceMap cmap = {0};

    build_convergence_map(
        &diags,
        chains,
        &cmap
    );

    render_convergence(&cmap);

    /* --- Minimal fix surface --- */

    FixSurface fs = build_fix_surface(&cmap);
    render_fix_surface(&fs);

    arena_destroy(&arena);
    return 0;
}
