#ifndef LIMINAL_SCOPE_GRAPH_H
#define LIMINAL_SCOPE_GRAPH_H

#include <stdint.h>
#include <stddef.h>

typedef struct ScopeNode {
    uint64_t scope_id;
    uint64_t parent_id;     /* 0 = root */
    uint64_t enter_time;
    uint64_t exit_time;     /* UINT64_MAX if open */
} ScopeNode;

typedef struct ScopeGraph {
    ScopeNode *nodes;
    size_t count;
} ScopeGraph;

#endif /* LIMINAL_SCOPE_GRAPH_H */
