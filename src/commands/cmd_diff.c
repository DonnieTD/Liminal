#include <stdio.h>
#include <string.h>

#include "consumers/run_descriptor.h"
#include "consumers/run_artifact.h"
#include "consumers/semantic_diff.h"

int load_run(const RunDescriptor *, RunArtifact *);
void semantic_diff_render(
    const SemanticDiff *, size_t, FILE *
);

int cmd_diff(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr,
            "usage: liminal diff <run-A> <run-B>\n");
        return 1;
    }

    RunDescriptor a = {
        .root_dir = argv[0],
        .run_id = argv[0],
        .meta_path = ".liminal/meta.json",
        .diagnostics_path = ".liminal/diagnostics.ndjson",
        .timeline_path = ".liminal/timeline.ndjson"
    };

    RunDescriptor b = {
        .root_dir = argv[1],
        .run_id = argv[1],
        .meta_path = ".liminal/meta.json",
        .diagnostics_path = ".liminal/diagnostics.ndjson",
        .timeline_path = ".liminal/timeline.ndjson"
    };

    RunArtifact ra = {0};
    RunArtifact rb = {0};

    if (load_run(&a, &ra) != 0 ||
        load_run(&b, &rb) != 0) {
        fprintf(stderr, "failed to load runs\n");
        return 1;
    }

    SemanticDiff diffs[256];
    size_t n = semantic_diff(
        &ra.diagnostics,
        &rb.diagnostics,
        diffs,
        256
    );

    semantic_diff_render(diffs, n, stdout);
    return 0;
}
