#include <stdint.h>
#ifndef LIMINAL_SCOPE_H
#define LIMINAL_SCOPE_H

/*
 * Scope
 *
 * A Scope represents a lexical environment.
 *
 * Scopes are immutable once created.
 * New scopes are created by pointing to a parent scope.
 *
 * Scope lookup is purely structural:
 *   - if a name exists in the current scope, it is in scope
 *   - otherwise, lookup proceeds to the parent
 */

struct HashMap;

typedef struct Scope {
     uint64_t id;
     
    /* Parent lexical scope (NULL for root / file scope) */
    struct Scope *parent;

    /* Bindings for this scope: name -> storage location */
    struct HashMap *bindings;
} Scope;

#endif /* LIMINAL_SCOPE_H */
