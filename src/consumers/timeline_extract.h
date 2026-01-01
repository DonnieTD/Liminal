#ifndef LIMINAL_TIMELINE_EXTRACT_H
#define LIMINAL_TIMELINE_EXTRACT_H

#include <stddef.h>
#include "executor/world.h"
#include "consumers/timeline_event.h"

size_t timeline_extract(
    const struct World *head,
    TimelineEvent *out,
    size_t cap
);

#endif
