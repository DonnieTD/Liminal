#ifndef LIMINAL_LIFETIME_H
#define LIMINAL_LIFETIME_H

#include <stdint.h>
#include <stddef.h>

struct World;

typedef struct ScopeLifetime {
    uint64_t scope_id;
    uint64_t enter_time;
    uint64_t exit_time;   /* UINT64_MAX means "still open" */
    void    *enter_origin;
    void    *exit_origin;
} ScopeLifetime;

/*
 * Collect scope lifetimes into `out`.
 *
 * Returns number of lifetimes written (<= cap).
 * If cap is too small, it truncates (for now).
 */
size_t lifetime_collect_scopes(struct World *head,
                               ScopeLifetime *out,
                               size_t cap);

#endif /* LIMINAL_LIFETIME_H */
