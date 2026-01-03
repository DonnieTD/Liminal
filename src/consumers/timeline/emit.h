#ifndef LIMINAL_TIMELINE_EMIT_H
#define LIMINAL_TIMELINE_EMIT_H

#include <stdio.h>

struct World;

/* Human / tool readable timeline */
void emit_timeline(
    const struct World *head,
    FILE *out
);

/* NDJSON artifact timeline */
void timeline_emit_ndjson(
    const struct World *head,
    FILE *out
);

#endif /* LIMINAL_TIMELINE_EMIT_H */
