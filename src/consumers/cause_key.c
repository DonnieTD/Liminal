#include "cause_key.h"

inline int cause_key_equal(const CauseKey *a, const CauseKey *b)
{
    return a->step == b->step &&
           a->ast_id == b->ast_id &&
           a->scope_id == b->scope_id;
}
