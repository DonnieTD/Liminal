#include <stdio.h>
#include <string.h>

#include "executor/universe.h"
#include "executor/world.h"
#include "executor/step.h"
#include "executor/scope.h"

#include "analyzer/trace.h"
#include "analyzer/use.h"

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
    ASTNode ast_func   = { .kind = AST_FUNCTION,  .id = 1, .line = 1 };
    ASTNode ast_decl_x = { .kind = AST_STATEMENT, .id = 2, .line = 2 };
    ASTNode ast_decl_y = { .kind = AST_STATEMENT, .id = 3, .line = 3 };
    ASTNode ast_use_x1 = { .kind = AST_STATEMENT, .id = 4, .line = 4 };
    ASTNode ast_use_y  = { .kind = AST_STATEMENT, .id = 5, .line = 5 };
    ASTNode ast_use_x2 = { .kind = AST_STATEMENT, .id = 6, .line = 6 };

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

    /* Enter function scope */
    universe_enter_scope(u, &ast_func);

    /* Declare x */
    universe_declare_variable(u, "x", &ast_decl_x);

    /* Declare y */
    universe_declare_variable(u, "y", &ast_decl_y);

    /* Valid use of x */
    universe_use_variable(u, "x", &ast_use_x1);

    /* Valid use of y */
    universe_use_variable(u, "y", &ast_use_y);

    /* Exit function scope */
    universe_exit_scope(u, &ast_func);

    /* Use-after-scope (x) */
    universe_use_variable(u, "x", &ast_use_x2);

    /* ---- RAW TIMELINE ---- */

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

    /* ---- TRACE FORWARD ---- */

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

    /* ---- TRACE BACKWARD ---- */

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

    /* ---- SCOPE LIFETIMES ---- */

    printf("\n-- SCOPE LIFETIMES --\n");

    ScopeLifetime lifetimes[64];
    size_t scope_count =
        lifetime_collect_scopes(u->head, lifetimes, 64);

    for (size_t i = 0; i < scope_count; i++) {
        const ScopeLifetime *lt = &lifetimes[i];

        if (lt->exit_time == UINT64_MAX) {
            printf(
                "scope=%llu enter=%llu exit=OPEN\n",
                (unsigned long long)lt->scope_id,
                (unsigned long long)lt->enter_time
            );
        } else {
            printf(
                "scope=%llu enter=%llu exit=%llu\n",
                (unsigned long long)lt->scope_id,
                (unsigned long long)lt->enter_time,
                (unsigned long long)lt->exit_time
            );
        }
    }

    /* ---- STEP_USE VALIDATION ---- */

    printf("\n-- USE VALIDATION --\n");

    UseReport uses[64];
    size_t use_count =
        analyze_step_use(
            u->head,
            lifetimes,
            scope_count,
            uses,
            64
        );

    for (size_t i = 0; i < use_count; i++) {
        const UseReport *r = &uses[i];

        const char *label =
            (r->kind == USE_OK) ? "OK" :
            (r->kind == USE_BEFORE_DECLARE) ? "use-before-declare" :
            "use-after-scope";

        printf(
            "t=%llu scope=%llu storage=%s => %s\n",
            (unsigned long long)r->time,
            (unsigned long long)r->scope_id,
            (r->storage_id == UINT64_MAX) ? "UNRESOLVED" : "RESOLVED",
            label
        );
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
