#ifndef LIMINAL_TIMELINE_EVENT_H
#define LIMINAL_TIMELINE_EVENT_H

#include <stdint.h>

typedef struct TimelineEvent {
    uint64_t time;
    uint32_t step_kind;
    uint32_t ast_id;
} TimelineEvent;

#endif /* LIMINAL_TIMELINE_EVENT_H */
