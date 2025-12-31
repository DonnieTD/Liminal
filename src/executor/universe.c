#include <stdlib.h>
#include "executor/universe.h"
#include "executor/world.h"

/*
 * Create an empty Universe.
 *
 * The Universe owns time and the World chain.
 * Initially, there are no Worlds.
 */
Universe *universe_create(void)
{
    Universe *u = calloc(1, sizeof(Universe));
    if (!u) {
        return NULL;
    }

    u->current_time = 0;
    u->head = NULL;
    u->tail = NULL;
    u->current = NULL;

    return u;
}
