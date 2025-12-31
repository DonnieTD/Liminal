#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "analyzer/diagnostic.h"
#include "analyzer/diagnostic_serialize.h"

int diagnostic_deserialize_line(FILE *in, Diagnostic *out)
{
    unsigned long long id;
    unsigned kind;
    unsigned long long time;
    unsigned long long scope;
    unsigned long long prev_scope;

    int n = fscanf(
        in,
        "{"
        "\"id\":\"%llx\","
        "\"kind\":%u,"
        "\"time\":%llu,"
        "\"scope\":%llu,"
        "\"prev_scope\":%llu"
        "}",
        &id,
        &kind,
        &time,
        &scope,
        &prev_scope
    );

    if (n != 5)
        return 0;

    /* consume remainder of line */
    int c;
    while ((c = fgetc(in)) != '\n' && c != EOF) {}

    out->id.value     = (uint64_t)id;
    out->kind         = (DiagnosticKind)kind;
    out->time         = (uint64_t)time;
    out->scope_id     = (uint64_t)scope;
    out->prev_scope   = (uint64_t)prev_scope;
    out->anchor       = NULL; /* reconstructed later */

    return 1;
}
