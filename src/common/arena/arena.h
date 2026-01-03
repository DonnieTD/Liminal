#ifndef LIMINAL_ARENA_H
#define LIMINAL_ARENA_H

#include <stddef.h>

typedef struct Arena {
    unsigned char *base;
    size_t capacity;
    size_t offset;
} Arena;

void arena_init(Arena *a, size_t capacity);
void *arena_alloc(Arena *a, size_t size);
void arena_reset(Arena *a);
void arena_destroy(Arena *a);

#endif
