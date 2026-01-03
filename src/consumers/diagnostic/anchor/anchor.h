#ifndef LIMINAL_DIAGNOSTIC_ANCHOR_H
#define LIMINAL_DIAGNOSTIC_ANCHOR_H

#include "analyzer/analyzer.h"
#include "../../timeline/timeline.h"

typedef struct DiagnosticAnchor {
    DiagnosticId id;
    uint64_t diagnostic_time;
    TimelineEvent cause;
} DiagnosticAnchor;

#endif
