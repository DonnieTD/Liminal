#ifndef LIMINAL_UNIVERSE_H
#define LIMINAL_UNIVERSE_H

#include <stdint.h>
#include "executor/world.h"

/*
 * Universe
 *
 * The Universe owns time and history.
 *
 * It is responsible for:
 * - creating Worlds
 * - linking Worlds into a timeline
 * - tracking the current World
 *
 * The Universe does NOT:
 * - execute semantics
 * - analyze Worlds
 * - mutate existing Worlds
 */

typedef struct Universe {
    uint64_t current_time;

    World *head;
    World *tail;
    World *current;
} Universe;

#endif /* LIMINAL_UNIVERSE_H */
