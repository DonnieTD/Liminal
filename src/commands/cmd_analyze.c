#include <stdio.h>
#include <stdlib.h>

#include "common/arena.h"
#include "executor/world.h"

#include "analyzer/diagnostic.h"

#include "consumers/root_chain.h"
#include "consumers/root_chain_role.h"
#include "consumers/convergence_map.h"
#include "consumers/fix_surface.h"

/* Forward declarations for render/build functions
 * (because you only have .c files, no headers)
 */
void render_convergence(const ConvergenceMap *m);
void render_fix_surface(const FixSurface *fs);

int build_convergence_map(
    const DiagnosticArtifact *diags,
    const RootChain *chains,
    ConvergenceMap *out
);

FixSurface build_fix_surface(const ConvergenceMap *map);

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
int cmd_analyze(const World *world)
{
    if (!world) {
        fprintf(stderr, "analyze: no world provided\n");
        return 1;
    }

    /* --- Diagnostics --- */

    DiagnosticArtifact diags =
        analyze_diagnostics((World *)world);

    if (diags.count == 0) {
        printf("No diagnostics.\n");
        return 0;
    }

    /* --- Arena for derived artifacts --- */

    Arena arena;
    arena_init(&arena, 32 * 1024);

    /* --- Root chains --- */

    RootChain *chains =
        arena_alloc(&arena, diags.count * sizeof(RootChain));

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
