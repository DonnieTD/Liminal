#include "executor/executor.h"
#include "common/common.h"

int scope_has_name(Scope *s, const char *name)
{
    return s && s->bindings && hashmap_get(s->bindings, name);
}