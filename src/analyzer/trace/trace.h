#ifndef LIMINAL_TRACE_H
#define LIMINAL_TRACE_H

#include <stdint.h>

struct World;

/*
 * Trace
 *
 * A read-only cursor over a sequence of Worlds.
 *
 * The Trace never mutates Worlds.
 * It may move forward and backward in time.
 */
typedef struct Trace {
    struct World *current;
} Trace;

/* Construction */
Trace trace_begin(struct World *head);
Trace trace_end(struct World *tail);

/* Navigation */
struct World *trace_current(Trace *t);
struct World *trace_next(Trace *t);
struct World *trace_prev(Trace *t);

/* Utility */
int trace_is_valid(const Trace *t);

#endif /* LIMINAL_TRACE_H */
