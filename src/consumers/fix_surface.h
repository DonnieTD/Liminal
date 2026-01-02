#ifndef LIMINAL_CONSUMERS_FIX_SURFACE_H
#define LIMINAL_CONSUMERS_FIX_SURFACE_H

#include <stddef.h>
#include "consumers/cause_key.h"

typedef struct {
    CauseKey *causes;
    size_t count;
    size_t capacity;
} FixSurface;

#endif /* LIMINAL_CONSUMERS_FIX_SURFACE_H */
