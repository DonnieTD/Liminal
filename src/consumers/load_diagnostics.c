#include "analyzer/diagnostic.h"
#include "analyzer/diagnostic_serialize.h"
#include <stdio.h>
#include <stdlib.h>

int load_diagnostics(const char *path, DiagnosticArtifact *out)
{
    if (!path || !out)
        return 1;

    FILE *f = fopen(path, "r");
    if (!f)
        return 2;

    Diagnostic *buf = calloc(256, sizeof(Diagnostic));
    size_t count = 0;

    while (count < 256 &&
           diagnostic_deserialize_line(f, &buf[count])) {
        count++;
    }

    fclose(f);

    out->items = buf;
    out->count = count;
    return 0;
}
