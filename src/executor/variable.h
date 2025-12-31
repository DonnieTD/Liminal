#ifndef LIMINAL_VARIABLE_H
#define LIMINAL_VARIABLE_H

#include <stdint.h>

/*
 * Variable
 *
 * A Variable represents a named storage identity.
 * It exists within a specific lexical scope.
 *
 * Variables are immutable.
 */
typedef struct Variable {
    uint64_t id;          /* unique id */
    uint64_t scope_id;    /* scope that owns this variable */
    const char *name;     /* optional (frontend later) */
} Variable;

#endif /* LIMINAL_VARIABLE_H */
