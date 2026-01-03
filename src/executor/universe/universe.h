#ifndef LIMINAL_UNIVERSE_H
#define LIMINAL_UNIVERSE_H

#include <stdint.h>
#include "../world/world.h"
#include "../../common/common.h"

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

    Arena world_arena;
    Arena step_arena;
    Arena scope_arena;
    Arena var_arena;
    Arena storage_arena;

    /* Identity counters */
    uint64_t next_scope_id;
    uint64_t next_var_id;
    uint64_t next_storage_id;
} Universe;


World *universe_step(Universe *u, void *origin);

void universe_attach_initial_world(Universe *u, World *w);

Universe *universe_create(void);

/* Scope control */
World *universe_enter_scope(Universe *u, void *origin);
World *universe_exit_scope(Universe *u, void *origin);


/* Variable operations */
World *universe_declare_variable(
    Universe *u,
    const char *name,
    void *origin
);

World *universe_use_variable(
    Universe *u,
    const char *name,
    void *origin
);

#endif /* LIMINAL_UNIVERSE_H */
