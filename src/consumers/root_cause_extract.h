#ifndef LIMINAL_ROOT_CAUSE_EXTRACT_H
#define LIMINAL_ROOT_CAUSE_EXTRACT_H

#include "executor/world.h"
#include "analyzer/diagnostic.h"
#include "consumers/root_cause.h"

RootCause root_cause_extract(
    const struct World *head,
    const struct Diagnostic *d
);

#endif
