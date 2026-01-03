#ifndef LIMINAL_STORAGE_H
#define LIMINAL_STORAGE_H

#include <stdint.h>

/*
 * Storage
 *
 * Represents a concrete storage location created by a declaration.
 * Storage has identity and lifetime, but no value yet.
 */
typedef struct Storage {
    uint64_t id;
    uint64_t declared_at;
} Storage;

#endif /* LIMINAL_STORAGE_H */
