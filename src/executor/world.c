#include <stdlib.h>
#include "executor/world.h"
#include "universe.h"

/*
 * Create the initial World.
 *
 * This represents time = 0.
 * No scopes, no stack, no memory, no step.
 */
World *world_create_initial(void)
{
    World *w = calloc(1, sizeof(World));
    if (!w) {
        return NULL;
    }

    w->time = 0;

    w->active_scope = NULL;
    w->call_stack = NULL;
    w->memory = NULL;
    w->step = NULL;

    w->prev = NULL;
    w->next = NULL;

    return w;
}

void universe_attach_initial_world(Universe *u, World *w)
{
    if (!u || !w) return;

    u->head = w;
    u->tail = w;
    u->current = w;
    u->current_time = w->time;
}

