#include "consumers/run_descriptor.h"
#include "consumers/run_artifact.h"

int run_probe(const RunDescriptor *rd);
int load_diagnostics(const char *path, DiagnosticArtifact *out);
int load_timeline(const char *path, void **out, size_t *count);

int load_run(const RunDescriptor *rd, RunArtifact *out)
{
    if (!rd || !out)
        return 1;

    int rc = run_probe(rd);
    if (rc != 0)
        return rc;

    out->run_id = (char *)rd->run_id;

    if (load_diagnostics(rd->diagnostics_path,
                          &out->diagnostics) != 0)
        return 10;

    /* Timeline optional */
    if (rd->timeline_path) {
        load_timeline(
            rd->timeline_path,
            (void **)&out->timeline,
            &out->timeline_count
        );
    }

    return 0;
}
