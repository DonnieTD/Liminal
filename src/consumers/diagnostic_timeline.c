#include <stdio.h>
#include "analyzer/diagnostic.h"
#include "analyzer/diagnostic_serialize.h"

void diagnostic_timeline_render(FILE *in, FILE *out)
{
    Diagnostic d;

    fprintf(out,
        "%-5s %-24s %-5s %s\n",
        "TIME", "KIND", "SCOPE", "ID"
    );
    fprintf(out,
        "%-5s %-24s %-5s %s\n",
        "----", "-----------------------", "-----",
        "----------------"
    );

    while (diagnostic_deserialize_line(in, &d)) {
        fprintf(
            out,
            "%5llu %-24s %5llu %016llx\n",
            (unsigned long long)d.time,
            diagnostic_kind_name(d.kind),
            (unsigned long long)d.scope_id,
            (unsigned long long)d.id.value
        );
    }
}
