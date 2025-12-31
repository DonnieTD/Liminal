#include <stdio.h>
#include "executor/executor.h"
#include "executor/world.h"
#include "executor/step.h"
#include "frontends/c/ast.h"

/* Forward */
static void exec_node(Universe *u,
                      const ASTProgram *p,
                      uint32_t node_id);

/* Entry point */
Universe *executor_build(const ASTProgram *p)
{
    if (!p || p->root_id == 0)
        return NULL;

    Universe *u = universe_create();
    if (!u)
        return NULL;

    World *w0 = world_create_initial(u);
    universe_attach_initial_world(u, w0);

    exec_node(u, p, p->root_id);
    return u;
}

/* Recursive structural traversal */
static void exec_node(Universe *u,
                      const ASTProgram *p,
                      uint32_t node_id)
{
    ASTNode *n = ast_node_get((ASTProgram *)p, node_id);
    if (!n) return;

    switch (n->kind) {

    case AST_PROGRAM:
        u->current = universe_step(u, n);
        u->current->step->kind = STEP_ENTER_PROGRAM;

        /* Assume single function for now */
        for (size_t i = 0; i < p->count; i++) {
            if (p->nodes[i].kind == AST_FUNCTION) {
                exec_node(u, p, p->nodes[i].id);
            }
        }

        u->current = universe_step(u, n);
        u->current->step->kind = STEP_EXIT_PROGRAM;
        break;

    case AST_FUNCTION:
        /* Structural marker */
        universe_step(u, n)->step->kind = STEP_ENTER_FUNCTION;

        /* Function introduces a scope */
        universe_enter_scope(u, n);

        /* Execute function body */
        exec_node(u, p, n->as.fn.body_id);

        /* Exit function scope */
        universe_exit_scope(u, n);

        /* Structural marker */
        universe_step(u, n)->step->kind = STEP_EXIT_FUNCTION;
        break;

    case AST_BLOCK:
        /* ENTER_SCOPE */
        universe_enter_scope(u, n);

        /* Execute statements */
        for (size_t i = 0; i < n->as.block.stmt_count; i++) {
            exec_node(u, p, n->as.block.stmt_ids[i]);
        }

        /* EXIT_SCOPE */
        universe_exit_scope(u, n);
        break;

    case AST_RETURN:
        u->current = universe_step(u, n);
        u->current->step->kind = STEP_RETURN;
        break;
    case AST_VAR_DECL:
        universe_declare_variable(
            u,
            n->as.vdecl.name,
            n
        );
        break;
    case AST_VAR_USE:
        universe_use_variable(
            u,
            n->as.vuse.name,
            n
        );
        break;
    default:
        /* Ignore unsupported nodes */
        break;
    }
}


/*
    Dump execution artifact (read-only)
    By convention, WORLD[1] is the initial world.
    How:
        1. Iterate worlds in order
        2. Print step info
        3. Print relevant metadata
        4. Done
*/
void executor_dump(const Universe *u)
{
    if (!u || !u->head) {
        printf("\n-- EXECUTION ARTIFACT --\n(empty)\n");
        return;
    }

    printf("\n-- EXECUTION ARTIFACT --\n");
    printf("world_count=%llu\n\n",
           (unsigned long long)u->current_time + 1);

    printf("WORLD[1]\n");

    for (World *w = u->head; w; w = w->next) {
        Step *s = w->step;
        if (!s) continue;

        printf("  STEP[%llu] ",
               (unsigned long long)w->time);

        switch (s->kind) {
        case STEP_ENTER_PROGRAM:  printf("ENTER_PROGRAM");  break;
        case STEP_EXIT_PROGRAM:   printf("EXIT_PROGRAM");   break;
        case STEP_ENTER_FUNCTION: printf("ENTER_FUNCTION"); break;
        case STEP_EXIT_FUNCTION:  printf("EXIT_FUNCTION");  break;
        case STEP_ENTER_SCOPE:    printf("ENTER_SCOPE");    break;
        case STEP_EXIT_SCOPE:     printf("EXIT_SCOPE");     break;
        case STEP_RETURN:         printf("RETURN");         break;
        case STEP_DECLARE:        printf("DECLARE");        break;
        case STEP_USE:            printf("USE");            break;
        default:                  printf("UNKNOWN");        break;
        }

        if (s->origin) {
            ASTNode *n = (ASTNode *)s->origin;
            printf(" ast=%u", n->id);
        }

        if (s->kind == STEP_DECLARE || s->kind == STEP_USE) {
            printf(" storage=%llu",
                   (unsigned long long)s->info);
        }

        printf("\n");
    }
}
