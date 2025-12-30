#include <stdio.h>
#include <string.h>

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
 * Intended implementation order (do NOT skip steps):
 *
 * 1. Frontend (src/frontends/c/)
 *    - lexer.c     : tokenize input source
 *    - parser.c   : build AST
 *    - ast.c      : own AST node structures
 *
 * 2. Executor (src/executor/)
 *    - universe.c : create initial Universe
 *    - world.c    : create initial empty World
 *    - step.c     : define execution step semantics
 *    - scope.c    : manage scope creation and lookup
 *    - stack.c    : manage call stack frames
 *    - memory.c   : manage storage, arenas, and aliasing
 *
 * 3. Execution loop
 *    - walk AST deterministically
 *    - for each semantic step:
 *        - create a new World
 *        - link it to previous World
 *        - record the AST node that caused the transition
 *
 * 4. Persistence (later)
 *    - serialize World sequence to disk as artifacts
 *
 * At the end of this command, execution is DONE.
 * No analysis happens here.
 */
static int cmd_run(const char *path)
{
    (void)path;
    printf("run: coming soon\n");
    return 0;
}

/*
 * Command: analyze
 *
 * Intended implementation order:
 *
 * 1. Load artifacts produced by `run`
 *    - World sequence
 *    - metadata
 *
 * 2. Analyzer (src/analyzer/)
 *    - trace.c     : iterate World sequence
 *    - lifetime.c : derive lifetime information
 *    - invariants : detect drift, violations, patterns
 *
 * 3. Artifact generation
 *    - emit JSON / structured data
 *    - no execution
 *    - no mutation of Worlds
 *
 * Analyzer code must NEVER:
 *    - execute semantics
 *    - modify Worlds
 *    - infer alternative paths
 *
 * Analysis is pure and replayable.
 */
static int cmd_analyze(const char *path)
{
    (void)path;
    printf("analyze: coming soon\n");
    return 0;
}

int main(int argc, char **argv)
{
    /*
     * CLI parsing only.
     * This function should remain readable in one screen.
     */

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
