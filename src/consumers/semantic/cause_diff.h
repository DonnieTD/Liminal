#ifndef LIMINAL_SEMANTIC_CAUSE_DIFF_H
#define LIMINAL_SEMANTIC_CAUSE_DIFF_H

#include "diff.h"

typedef enum CauseChangeKind {
    CAUSE_NONE,
    CAUSE_NEW_STEP,
    CAUSE_STEP_MOVED,
    CAUSE_STEP_REMOVED
} CauseChangeKind;

typedef struct SemanticCauseDiff {
    SemanticDiff base;
    CauseChangeKind cause;
    uint32_t old_step;
    uint32_t new_step;
} SemanticCauseDiff;

#endif
