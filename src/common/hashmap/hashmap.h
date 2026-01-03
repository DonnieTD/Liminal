#ifndef LIMINAL_HASHMAP_H
#define LIMINAL_HASHMAP_H

#include <stddef.h>

struct Arena;

/*
 * HashMap
 *
 * Minimal string-key hashmap.
 * Keys are NOT owned.
 * Values are opaque pointers.
 *
 * Allocation is arena-backed and monotonic.
 */
typedef struct HashMap HashMap;

/* Create an empty hashmap */
HashMap *hashmap_create(struct Arena *arena, size_t bucket_count);

/* Shallow clone (structural copy of buckets) */
HashMap *hashmap_clone(HashMap *src, struct Arena *arena);

/* Insert or overwrite */
void hashmap_put(HashMap *map, const char *key, void *value);

/* Lookup (NULL if missing) */
void *hashmap_get(HashMap *map, const char *key);

#endif /* LIMINAL_HASHMAP_H */
