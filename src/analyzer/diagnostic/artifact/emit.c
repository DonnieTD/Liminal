#include "analyzer/analyzer.h"
#include "common/common.h"

#include <stdio.h>
#include <string.h>
#include "consumers/consumers.h"
#include "executor/executor.h"


static void emit_meta(const ArtifactContext *ctx, const char *dir)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/meta.json", dir);

    char buf[1024];
    snprintf(
        buf, sizeof(buf),
        "{\n"
        "  \"liminal_version\": \"0.5.3\",\n"
        "  \"run_id\": \"%s\",\n"
        "  \"started_at\": %lu,\n"
        "  \"input\": \"%s\"\n"
        "}\n",
        ctx->run_id,
        ctx->started_at,
        ctx->input_path
    );

    fs_write_file(path, buf, strlen(buf));
}

static void emit_diagnostics(
    const DiagnosticArtifact *a,
    const char *dir
)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/diagnostics.ndjson", dir);

    FILE *out = fs_open_file(path);
    if (!out)
        return;

    diagnostic_project_ndjson(a, out);
    fclose(out);
}

void artifact_emit_all(
    const ArtifactContext *ctx,
    const DiagnosticArtifact *diagnostics
)
{
    char run_dir[512];

    fs_mkdir_if_missing(ctx->root);
    snprintf(run_dir, sizeof(run_dir), "%s/%s", ctx->root, ctx->run_id);
    fs_mkdir_if_missing(run_dir);

    emit_meta(ctx, run_dir);
    emit_diagnostics(diagnostics, run_dir);

    /* Timeline emission (first-class artifact) */
    {
        char path[512];
        snprintf(path, sizeof(path), "%s/timeline.ndjson", run_dir);
        FILE *out = fs_open_file(path);
        if (out) {
            timeline_emit_ndjson(ctx->world_head, out);
            fclose(out);
        }
    }
}

