#include "analyzer/trace.h"
#include "executor/world.h"
#include <stdlib.h>

/*
 * Create a Trace starting at the beginning of time.
 */
Trace trace_begin(struct World *head)
{
    Trace t;
    t.current = head;
    return t;
}

/*
 * Create a Trace starting at the end of time.
 */
Trace trace_end(struct World *tail)
{
    Trace t;
    t.current = tail;
    return t;
}

/*
 * Get the current World.
 */
struct World *trace_current(Trace *t)
{
    if (!t) {
        return NULL;
    }
    return t->current;
}

/*
 * Advance the Trace forward in time.
 */
struct World *trace_next(Trace *t)
{
    if (!t || !t->current) {
        return NULL;
    }

    t->current = t->current->next;
    return t->current;
}

/*
 * Move the Trace backward in time.
 */
struct World *trace_prev(Trace *t)
{
    if (!t || !t->current) {
        return NULL;
    }

    t->current = t->current->prev;
    return t->current;
}

/*
 * Check whether the Trace points to a valid World.
 */
int trace_is_valid(const Trace *t)
{
    return t && t->current;
}
