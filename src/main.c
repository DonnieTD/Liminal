#include <stdio.h>
#include <string.h>

#include "executor/universe.h"
#include "executor/world.h"
#include "executor/step.h"
#include "executor/scope.h"

#include "analyzer/trace.h"
#include "analyzer/use.h"

#include "frontends/c/ast.h"
#include "frontends/c/frontend.h"

#include "analyzer/lifetime.h"
#include "executor/executor.h"


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
    if (!path) {
        fprintf(stderr, "error: no input file\n");
        return 1;
    }

    /* ---- FRONTEND: parse source -> AST artifact ---- */

    ASTProgram *ast = c_parse_file_to_ast(path);
    if (!ast) {
        fprintf(stderr, "failed to parse AST\n");
        return 1;
    }

    /* ---- FRONTEND ARTIFACT ---- */

    ast_dump(ast);

    /* ---- EXECUTOR: AST -> World timeline ---- */

    Universe *u = executor_build(ast);
    if (!u) {
        fprintf(stderr, "failed to build execution artifact\n");
        ast_program_free(ast);
        return 1;
    }

    /* ---- EXECUTION ARTIFACT ---- */

    executor_dump(u);

    /*
     * NOTE:
     * - Universe owns all executor memory via arenas
     * - No destruction yet (process-lifetime ownership is fine)
     * - Analysis is NOT invoked here
     */

    /* ---- CLEANUP (frontend only) ---- */

    ast_program_free(ast);

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
