#ifndef LIMINAL_SCOPE_ALIGNMENT_H
#define LIMINAL_SCOPE_ALIGNMENT_H

#include <stdint.h>

typedef enum ScopeChangeKind {
    SCOPE_UNCHANGED,
    SCOPE_ADDED,
    SCOPE_REMOVED,
    SCOPE_SPLIT,
    SCOPE_MERGED,
    SCOPE_MOVED
} ScopeChangeKind;

typedef struct ScopeAlignment {
    uint64_t old_sig;
    uint64_t new_sig;
    ScopeChangeKind kind;
} ScopeAlignment;

#endif
