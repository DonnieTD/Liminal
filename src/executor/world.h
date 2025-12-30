#ifndef LIMINAL_WORLD_H
#define LIMINAL_WORLD_H

#include <stdint.h>

/*
 * World
 *
 * A World represents the complete semantic state of the program
 * at a single moment in time.
 *
 * Worlds are immutable once created.
 * Execution advances by creating a new World and linking it
 * to the previous one.
 *
 * Worlds do not own history.
 * Worlds do not execute logic.
 * Worlds do not speculate.
 */

struct Scope;
struct CallStack;
struct Memory;
struct Step;

typedef struct World {
    /* Monotonic time index */
    uint64_t time;

    /* Semantic state */
    struct Scope     *active_scope;
    struct CallStack *call_stack;
    struct Memory    *memory;

    /* What caused this state */
    struct Step      *step;

    /* Temporal links */
    struct World     *prev;
    struct World     *next;
} World;

#endif /* LIMINAL_WORLD_H */
