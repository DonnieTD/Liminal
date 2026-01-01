#include <stddef.h>
#include <stdint.h>

#include "common/arena.h"

#include "executor/world.h"
#include "executor/step.h"

#include "consumers/root_chain.h"
#include "consumers/root_chain_role.h"

#include "analyzer/diagnostic.h"
#include "frontends/c/ast.h"   /* for ASTNode */

/*
 * Build a root-cause chain by walking the World timeline backwards
 * from the diagnostic time.
 *
 * nodes[0] is the closest causal event to the diagnostic.
 */
RootChain build_root_chain(
    Arena *arena,
    const World *world_tail,
    const Diagnostic *diag
)
{
    RootChain chain = {0};
    chain.diagnostic_id = diag->id;

    /* ----------------------------------------
     * First pass: count causal steps
     * ---------------------------------------- */
    size_t count = 0;

    for (const World *w = world_tail; w; w = w->prev) {
        if (w->time > diag->time)
            continue;

        if (!w->step)
            continue;

        count++;
    }

    if (count == 0)
        return chain;

    /* ----------------------------------------
     * Allocate nodes
     * ---------------------------------------- */
    RootChainNode *nodes = arena_alloc(arena, count * sizeof(*nodes));
    if (!nodes)
        return chain;

    /* ----------------------------------------
     * Second pass: populate nodes
     * ---------------------------------------- */
    size_t i = 0;

    for (const World *w = world_tail; w; w = w->prev) {
        if (w->time > diag->time)
            continue;

        const Step *s = w->step;
        if (!s)
            continue;

        RootChainNode *n = &nodes[i++];

        n->time = w->time;
        n->step = s->kind;

        /* Derive AST id from origin */
        if (s->origin) {
            const struct ASTNode *ast = (const struct ASTNode *)s->origin;
            n->ast_id = ast->id;
        } else {
            n->ast_id = 0;
        }

        /* Derive scope id conservatively */
        switch (s->kind) {
            case STEP_ENTER_SCOPE:
            case STEP_EXIT_SCOPE:
                n->scope_id = s->info;
                break;

            default:
                n->scope_id = diag->scope_id;
                break;
        }

        n->role = ROOT_ROLE_WITNESS;
    }

    chain.nodes = nodes;
    chain.count = i;

    /* ----------------------------------------
     * Assign semantic roles
     * ---------------------------------------- */
    assign_root_chain_roles(&chain, diag->kind);

    return chain;
}
