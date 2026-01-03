#ifndef LIMINAL_VARIABLE_LIFETIME_H
#define LIMINAL_VARIABLE_LIFETIME_H

#include <stdint.h>
#include <stddef.h>

struct World;

typedef struct VariableLifetime {
    uint64_t var_id;
    uint64_t scope_id;
    uint64_t declare_time;
    uint64_t end_time;   /* scope exit */
} VariableLifetime;

size_t lifetime_collect_variables(struct World *head,
                                  VariableLifetime *out,
                                  size_t cap);

#endif
