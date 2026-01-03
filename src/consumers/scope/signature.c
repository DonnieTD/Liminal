#include "./signature.h"

uint64_t scope_signature(const ScopeNode *n)
{
    /*
     * Stable semantic signature:
     *
     * [ parent_id | enter_time | exit_time ]
     *
     * Scope ID deliberately excluded.
     */
    return
        (n->parent_id << 48) ^
        (n->enter_time << 24) ^
        (n->exit_time);
}
