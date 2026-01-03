#ifndef LIMINAL_WORLD_H
#define LIMINAL_WORLD_H

#include <stdint.h>

/*
 * World
 *
 * A World represents the complete semantic state of the program
 * at a single moment in time.
 */

struct Universe;   /* forward declaration */
struct Scope;
struct CallStack;
struct Memory;
struct Step;

typedef struct World {
    uint64_t time;

    struct Scope     *active_scope;
    struct CallStack *call_stack;
    struct Memory    *memory;
    struct Step      *step;

    struct World     *prev;
    struct World     *next;
} World;

/* Constructors */
World *world_create_initial(struct Universe *u);

/* Cloning */
World *world_clone(struct Universe *u, const World *src);

#endif /* LIMINAL_WORLD_H */
