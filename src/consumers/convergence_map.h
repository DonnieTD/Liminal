#ifndef LIMINAL_CONVERGENCE_MAP_H
#define LIMINAL_CONVERGENCE_MAP_H

#include <stddef.h>
#include "consumers/cause_key.h"
#include "analyzer/diagnostic.h"

/*
 * A convergence entry groups diagnostics that share
 * an identical semantic cause key.
 */
typedef struct ConvergenceEntry {
    CauseKey key;

    const Diagnostic **diagnostics;
    size_t count;
    size_t capacity;
} ConvergenceEntry;

/*
 * A convergence map groups all convergence entries
 * discovered in a run.
 */
typedef struct ConvergenceMap {
    ConvergenceEntry *entries;
    size_t count;
    size_t capacity;
} ConvergenceMap;

void convergence_map_add(
    ConvergenceMap *map,
    const CauseKey *key,
    const Diagnostic *diag
);

#endif /* LIMINAL_CONVERGENCE_MAP_H */
