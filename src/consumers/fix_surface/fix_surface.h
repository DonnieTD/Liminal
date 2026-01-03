#ifndef LIMINAL_CONSUMERS_FIX_SURFACE_H
#define LIMINAL_CONSUMERS_FIX_SURFACE_H

#include <stddef.h>
#include "../cause_key/cause_key.h"
#include "../convergence/map/map.h"

typedef struct {
    CauseKey *causes;
    size_t count;
    size_t capacity;
} FixSurface;

FixSurface build_fix_surface(const ConvergenceMap *map);


#endif /* LIMINAL_CONSUMERS_FIX_SURFACE_H */
