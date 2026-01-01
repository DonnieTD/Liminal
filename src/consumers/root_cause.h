#ifndef LIMINAL_ROOT_CAUSE_H
#define LIMINAL_ROOT_CAUSE_H

#include <stdint.h>

typedef enum RootCauseKind {
    ROOT_CAUSE_DECLARATION,
    ROOT_CAUSE_USE,
    ROOT_CAUSE_SCOPE_ENTRY,
    ROOT_CAUSE_SCOPE_EXIT,
    ROOT_CAUSE_PREVIOUS_DIAGNOSTIC,
    ROOT_CAUSE_UNKNOWN
} RootCauseKind;

typedef struct RootCause {
    RootCauseKind kind;
    uint64_t time;
    uint64_t ast_id;
    uint64_t scope_id;
} RootCause;

#endif
