#include <stdlib.h>
#include <string.h>

#include "consumers/convergence_map.h"
#include "consumers/cause_key.h"

/* Forward declaration — real definition lives in cause_key.c */
int cause_key_equal(const CauseKey *a, const CauseKey *b);

static ConvergenceEntry *
find_entry(ConvergenceMap *m, const CauseKey *key)
{
    for (size_t i = 0; i < m->count; i++) {
        if (cause_key_equal(&m->entries[i].key, key))
            return &m->entries[i];
    }
    return NULL;
}

void convergence_map_add(
    ConvergenceMap *m,
    const CauseKey *key,
    const Diagnostic *diag
)
{
    ConvergenceEntry *e = find_entry(m, key);

    if (!e) {
        if (m->count == m->capacity) {
            size_t nc = m->capacity ? m->capacity * 2 : 4;
            m->entries = realloc(m->entries, nc * sizeof(*m->entries));
            m->capacity = nc;
        }

        e = &m->entries[m->count++];
        memset(e, 0, sizeof(*e));
        e->key = *key;
    }

    if (e->count == e->capacity) {
        size_t nc = e->capacity ? e->capacity * 2 : 4;
        e->diagnostics = realloc(e->diagnostics, nc * sizeof(*e->diagnostics));
        e->capacity = nc;
    }

    e->diagnostics[e->count++] = diag;
}
