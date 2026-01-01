#include "consumers/run_artifact.h"

int validate_run(const RunArtifact *r)
{
    if (!r)
        return 1;

    /* diagnostics time monotonic */
    for (size_t i = 1; i < r->diagnostics.count; i++) {
        if (r->diagnostics.items[i].time <
            r->diagnostics.items[i - 1].time)
            return 2;
    }

    /* timeline monotonic */
    for (size_t i = 1; i < r->timeline_count; i++) {
        if (r->timeline[i].time <
            r->timeline[i - 1].time)
            return 3;
    }

    return 0;
}
