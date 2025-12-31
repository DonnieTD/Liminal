#include <stdlib.h>
#include "executor/world.h"
#include "executor/universe.h"
#include "executor/step.h"

/*
 * Create the initial World.
 *
 * This represents time = 0.
 * No scopes, no stack, no memory.
 */
World *world_create_initial(Universe *u)
{
    if (!u) {
        return NULL;
    }

    World *w = arena_alloc(&u->world_arena, sizeof(World));
    if (!w) {
        return NULL;
    }

    w->time = 0;
    w->active_scope = NULL;
    w->call_stack   = NULL;
    w->memory       = NULL;
    w->step         = NULL;

    w->prev = NULL;
    w->next = NULL;

    return w;
}
/*
 * Clone a World.
 *
 * This performs a shallow copy of semantic state.
 * Ownership remains with the Universe.
 *
 * Time, links, and causality are adjusted by the caller.
 */
World *world_clone(Universe *u, const World *src)
{
    if (!u || !src) {
        return NULL;
    }

    World *w = arena_alloc(&u->world_arena, sizeof(World));
    if (!w) {
        return NULL;
    }

    w->time         = src->time;
    w->active_scope = src->active_scope;
    w->call_stack   = src->call_stack;
    w->memory       = src->memory;
    w->step         = src->step;

    w->prev = NULL;
    w->next = NULL;

    return w;
}
