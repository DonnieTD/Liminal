#include <stdio.h>
#include <string.h>

#include "executor/universe.h"
#include "executor/world.h"
#include "executor/step.h"
#include "executor/scope.h"

#include "analyzer/trace.h"
#include "frontends/c/ast.h"

#include "analyzer/lifetime.h"


/*
 * Liminal CLI entry point
 *
 * This file is intentionally thin.
 * It exists only to orchestrate the high-level pipeline:
 *
 *   source -> AST -> Worlds -> Analysis -> Artifacts
 *
 * No semantic logic belongs here.
 * No execution details belong here.
 * No analysis logic belongs here.
 *
 * This file should remain boring forever.
 */

static void print_usage(const char *prog)
{
    printf("Usage: %s <command> [args]\n", prog);
    printf("\n");
    printf("Commands:\n");
    printf("  run <file>        Execute a program into a world delta set\n");
    printf("  analyze <path>    Analyze world delta sets\n");
    printf("\n");
}

/*
 * Command: run
 *
 * This is a *demonstration scaffold*.
 * No real parsing or execution yet.
 */
static int cmd_run(const char *path)
{
    (void)path;

    /* Dummy AST nodes (identity only) */
    ASTNode ast_func  = { .kind = AST_FUNCTION,  .id = 1, .line = 1 };
    ASTNode ast_stmt1 = { .kind = AST_STATEMENT, .id = 2, .line = 2 };
    ASTNode ast_stmt2 = { .kind = AST_STATEMENT, .id = 3, .line = 3 };

    /* Create Universe */
    Universe *u = universe_create();
    if (!u) {
        fprintf(stderr, "failed to create universe\n");
        return 1;
    }

    /* Create initial World */
    World *w0 = world_create_initial(u);
    if (!w0) {
        fprintf(stderr, "failed to create initial world\n");
        return 1;
    }

    universe_attach_initial_world(u, w0);

    /* ---- Simulated program ---- */

    universe_enter_scope(u, &ast_func);
    universe_step(u, &ast_stmt1);
    universe_step(u, &ast_stmt2);
    universe_exit_scope(u, &ast_func);

    /* ---- Print raw timeline ---- */

    printf("\n-- RAW TIMELINE --\n");
    for (World *w = u->head; w; w = w->next) {
        printf(
            "t=%llu step=%d scope=%llu origin=%p\n",
            (unsigned long long)w->time,
            w->step ? w->step->kind : -1,
            w->active_scope ? (unsigned long long)w->active_scope->id : 0,
            w->step ? w->step->origin : NULL
        );
    }

    /* ---- Trace forward ---- */

    printf("\n-- TRACE FORWARD --\n");
    Trace t = trace_begin(u->head);
    while (trace_is_valid(&t)) {
        World *w = trace_current(&t);
        printf(
            "t=%llu step=%d scope=%llu\n",
            (unsigned long long)w->time,
            w->step ? w->step->kind : -1,
            w->active_scope ? (unsigned long long)w->active_scope->id : 0
        );
        trace_next(&t);
    }

    /* ---- Trace backward ---- */

    printf("\n-- TRACE BACKWARD --\n");
    t = trace_end(u->tail);
    while (trace_is_valid(&t)) {
        World *w = trace_current(&t);
        printf(
            "t=%llu step=%d scope=%llu\n",
            (unsigned long long)w->time,
            w->step ? w->step->kind : -1,
            w->active_scope ? (unsigned long long)w->active_scope->id : 0
        );
        trace_prev(&t);
    }

    printf("\n-- SCOPE LIFETIMES --\n");
    ScopeLifetime lifetimes[64];
    size_t count = lifetime_collect_scopes(u->head, lifetimes, 64);

    for (size_t i = 0; i < count; i++) {
        const ScopeLifetime *lt = &lifetimes[i];
        
        if (lt->exit_time == UINT64_MAX) {
            printf("scope=%llu enter=%llu exit=OPEN\n",
                (unsigned long long)lt->scope_id,
                (unsigned long long)lt->enter_time);
        } else {
            printf("scope=%llu enter=%llu exit=%llu\n",
                (unsigned long long)lt->scope_id,
                (unsigned long long)lt->enter_time,
                (unsigned long long)lt->exit_time);
        }
    }

    return 0;
}

/*
 * Command: analyze
 *
 * Placeholder for analysis pipeline.
 */
static int cmd_analyze(const char *path)
{
    (void)path;
    printf("analyze: coming soon\n");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage(argv[0]);
        printf("\ncoming soon\n");
        return 0;
    }

    if (strcmp(argv[1], "run") == 0) {
        if (argc < 3) {
            printf("error: missing input file\n");
            return 1;
        }
        return cmd_run(argv[2]);
    }

    if (strcmp(argv[1], "analyze") == 0) {
        if (argc < 3) {
            printf("error: missing artifact path\n");
            return 1;
        }
        return cmd_analyze(argv[2]);
    }

    printf("unknown command: %s\n", argv[1]);
    print_usage(argv[0]);
    printf("\ncoming soon\n");

    return 1;
}
