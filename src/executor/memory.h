#ifndef LIMINAL_MEMORY_H
#define LIMINAL_MEMORY_H

#include <stdint.h>

/*
 * Memory
 *
 * Represents the program's memory model.
 *
 * This is NOT raw bytes.
 * This is semantic storage with identity, lifetime, and aliasing.
 *
 * Memory objects are created, referenced, and destroyed over time.
 * Lifetime is determined by presence in the current World.
 */

struct Arena;
struct HashMap;

typedef struct Memory {
    /* Underlying allocation arenas */
    struct Arena *arena;

    /*
     * Pointer / alias map:
     *   logical address -> memory object
     *
     * Used to track aliasing, lifetimes, and invalid access.
     */
    struct HashMap *objects;
} Memory;

#endif /* LIMINAL_MEMORY_H */
