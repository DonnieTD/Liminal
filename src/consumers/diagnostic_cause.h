#ifndef LIMINAL_DIAGNOSTIC_CAUSE_H
#define LIMINAL_DIAGNOSTIC_CAUSE_H

#include "consumers/root_cause.h"

typedef struct DiagnosticCause {
    uint64_t diagnostic_id;
    RootCause cause;
} DiagnosticCause;

#endif
