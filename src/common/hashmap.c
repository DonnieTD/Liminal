#include "common/hashmap.h"
#include "common/arena.h"

#include <string.h>
#include <stdint.h>

typedef struct HashEntry {
    const char *key;
    void *value;
    struct HashEntry *next;
} HashEntry;

struct HashMap {
    struct Arena *arena;
    size_t bucket_count;
    HashEntry **buckets;
};

/* Simple FNV-1a hash */
static uint64_t hash_str(const char *s)
{
    uint64_t h = 1469598103934665603ULL;
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 1099511628211ULL;
    }
    return h;
}

HashMap *hashmap_create(struct Arena *arena, size_t bucket_count)
{
    if (!arena || bucket_count == 0) {
        return NULL;
    }

    HashMap *map = arena_alloc(arena, sizeof(HashMap));
    if (!map) {
        return NULL;
    }

    map->arena = arena;
    map->bucket_count = bucket_count;

    map->buckets = arena_alloc(arena, sizeof(HashEntry *) * bucket_count);
    if (!map->buckets) {
        return NULL;
    }

    memset(map->buckets, 0, sizeof(HashEntry *) * bucket_count);
    return map;
}

HashMap *hashmap_clone(HashMap *src, struct Arena *arena)
{
    if (!arena) {
        return NULL;
    }

    if (!src) {
        /* Create empty map if no parent */
        return hashmap_create(arena, 32);
    }

    HashMap *map = arena_alloc(arena, sizeof(HashMap));
    if (!map) {
        return NULL;
    }

    map->arena = arena;
    map->bucket_count = src->bucket_count;

    map->buckets = arena_alloc(arena, sizeof(HashEntry *) * map->bucket_count);
    if (!map->buckets) {
        return NULL;
    }

    /* Structural (shallow) copy */
    memcpy(
        map->buckets,
        src->buckets,
        sizeof(HashEntry *) * map->bucket_count
    );

    return map;
}

void hashmap_put(HashMap *map, const char *key, void *value)
{
    if (!map || !key) {
        return;
    }

    uint64_t h = hash_str(key);
    size_t idx = h % map->bucket_count;

    /* Overwrite if exists */
    for (HashEntry *e = map->buckets[idx]; e; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            e->value = value;
            return;
        }
    }

    /* Insert new entry */
    HashEntry *e = arena_alloc(map->arena, sizeof(HashEntry));
    if (!e) {
        return;
    }

    e->key = key;
    e->value = value;
    e->next = map->buckets[idx];

    map->buckets[idx] = e;
}

void *hashmap_get(HashMap *map, const char *key)
{
    if (!map || !key) {
        return NULL;
    }

    uint64_t h = hash_str(key);
    size_t idx = h % map->bucket_count;

    for (HashEntry *e = map->buckets[idx]; e; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            return e->value;
        }
    }

    return NULL;
}
