#ifndef LIMINAL_FIX_SURFACE_BUILD_H
#define LIMINAL_FIX_SURFACE_BUILD_H

#include "consumers/convergence_map.h"
#include "consumers/fix_surface.h"

/*
 * Extract minimal fix surface from convergence map.
 *
 * PURE:
 *  - deterministic
 *  - no mutation
 */
FixSurface build_fix_surface(const ConvergenceMap *map);

#endif /* LIMINAL_FIX_SURFACE_BUILD_H */
