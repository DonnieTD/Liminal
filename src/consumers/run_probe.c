#include "consumers/run_descriptor.h"
#include "consumers/run_contract.h"
#include <sys/stat.h>

static int file_exists(const char *path)
{
    struct stat st;
    return path && stat(path, &st) == 0;
}

int run_probe(const RunDescriptor *rd)
{
    if (!rd)
        return 1;

    if (LIMINAL_RUN_CONTRACT.require_meta &&
        !file_exists(rd->meta_path))
        return 2;

    if (LIMINAL_RUN_CONTRACT.require_diagnostics &&
        !file_exists(rd->diagnostics_path))
        return 3;

    if (!LIMINAL_RUN_CONTRACT.allow_missing_timeline &&
        !file_exists(rd->timeline_path))
        return 4;

    return 0;
}
