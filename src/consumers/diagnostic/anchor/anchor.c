#include "./anchor.h"

int anchor_diagnostic(
    const Diagnostic *d,
    const TimelineEvent *events,
    size_t event_count,
    DiagnosticAnchor *out
)
{
    if (!d || !events || !out)
        return 0;

    TimelineEvent best = {0};
    int found = 0;

    for (size_t i = 0; i < event_count; i++) {
        if (events[i].time <= d->time) {
            best = events[i];
            found = 1;
        } else {
            break;
        }
    }

    if (!found)
        return 0;

    *out = (DiagnosticAnchor){
        .id = d->id,
        .diagnostic_time = d->time,
        .cause = best
    };

    return 1;
}
