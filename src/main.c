#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "executor/universe.h"
#include "executor/world.h"
#include "executor/step.h"
#include "executor/scope.h"
#include "executor/executor.h"

#include "analyzer/trace.h"
#include "analyzer/use.h"
#include "analyzer/lifetime.h"
#include "analyzer/diagnostic.h"
#include "analyzer/constraint_engine.h"
#include "analyzer/artifact_emit.h"

#include "frontends/c/ast.h"
#include "frontends/c/frontend.h"

#include "commands/cmd_analyze.h"
#include "commands/cmd_policy.h"

#include "consumers/timeline_emit.h"

#include "policy/default_policy.h"

/*
 * Liminal CLI entry point
 *
 * Orchestration ONLY.
 * No semantics.
 * No execution logic.
 * No analysis logic.
 */

static void print_usage(const char *prog)
{
    printf("Usage: %s run <file> [options]\n", prog);
    printf("\nOptions:\n");
    printf("  --emit-artifacts\n");
    printf("  --emit-timeline\n");
    printf("  --artifact-dir <path>   (default: .liminal)\n");
    printf("  --run-id <string>       (optional override)\n");
    printf("\n");
}

static int cmd_run(int argc, char **argv)
{
    const char *input_path     = NULL;
    const char *artifact_root  = ".liminal";
    const char *run_id_override = NULL;

    bool emit_artifacts = false;
    bool emit_timeline_flag = false;

    /* ---- ARG PARSING ---- */
    for (int i = 0; i < argc; i++) {
        if (!input_path && argv[i][0] != '-') {
            input_path = argv[i];
            continue;
        }

        if (strcmp(argv[i], "--emit-artifacts") == 0) {
            emit_artifacts = true;
            continue;
        }

        if (strcmp(argv[i], "--emit-timeline") == 0) {
            emit_timeline_flag = true;
            continue;
        }

        if (strcmp(argv[i], "--artifact-dir") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: --artifact-dir requires a path\n");
                return 1;
            }
            artifact_root = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "--run-id") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: --run-id requires value\n");
                return 1;
            }
            run_id_override = argv[++i];
            continue;
        }
    }

    if (!input_path) {
        fprintf(stderr, "error: no input file\n");
        return 1;
    }

    /* ---- FRONTEND ---- */
    ASTProgram *ast = c_parse_file_to_ast(input_path);
    if (!ast) {
        fprintf(stderr, "failed to parse AST\n");
        return 1;
    }

    ast_dump(ast);

    /* ---- EXECUTOR ---- */
    Universe *u = executor_build(ast);
    if (!u) {
        fprintf(stderr, "failed to build execution artifact\n");
        ast_program_free(ast);
        return 1;
    }

    executor_dump(u);

    /* ---- ANALYSIS ---- */
    DiagnosticArtifact diagnostics = analyze_diagnostics(u->head);
    diagnostic_dump(&diagnostics);

    /* ---- POLICY (STAGE 6) ---- */
    if (cmd_apply_policy(&LIMINAL_DEFAULT_POLICY, &diagnostics) != 0) {
        ast_program_free(ast);
        return 1;
    }

    /* ---- ARTIFACT EMISSION ---- */
    if (emit_artifacts || emit_timeline_flag) {
        time_t now = time(NULL);
        char run_id[64];

        if (run_id_override) {
            snprintf(run_id, sizeof(run_id), "%s", run_id_override);
        } else {
            snprintf(run_id, sizeof(run_id), "run-%lu", (unsigned long)now);
        }

        ArtifactContext ctx = {
            .root       = artifact_root,
            .run_id     = run_id,
            .input_path = input_path,
            .started_at = (unsigned long)now,
            .world_head = u->head
        };

        if (emit_artifacts) {
            artifact_emit_all(&ctx, &diagnostics);
        }

        if (emit_timeline_flag) {
          emit_timeline(u->head, stdout);
        }
    }

    ast_program_free(ast);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "run") == 0) {
        return cmd_run(argc - 2, argv + 2);
    }

    if (strcmp(argv[1], "analyze") == 0) {
        if (argc < 3) {
            fprintf(stderr, "error: missing artifact path\n");
            return 1;
        }
        return cmd_analyze(argv[2]);
    }

    fprintf(stderr, "unknown command: %s\n", argv[1]);
    print_usage(argv[0]);
    return 1;
}
