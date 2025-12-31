#include <stdlib.h>
#include <string.h>
#include "common/arena.h"

void arena_init(Arena *a, size_t capacity)
{
    a->base = malloc(capacity);
    a->capacity = capacity;
    a->offset = 0;
}

void *arena_alloc(Arena *a, size_t size)
{
    /* align to 8 bytes */
    size = (size + 7) & ~7;

    if (a->offset + size > a->capacity) {
        return NULL;
    }

    void *ptr = a->base + a->offset;
    a->offset += size;

    memset(ptr, 0, size);
    return ptr;
}

void arena_reset(Arena *a)
{
    a->offset = 0;
}

void arena_destroy(Arena *a)
{
    free(a->base);
    a->base = NULL;
    a->capacity = 0;
    a->offset = 0;
}
