#include "analyzer/use_validate.h"
#include "executor/world.h"

/*
 * analyze_use_validation
 *
 * Step 3.x placeholder.
 *
 * This pass will eventually:
 *  - analyze STEP_USE events
 *  - correlate against variable + scope lifetimes
 *  - emit diagnostics
 *
 * For now:
 *  - it is a no-op
 *  - required only to satisfy the diagnostic pipeline
 */
size_t analyze_use_validation(
    struct World *head,
    Diagnostic *out,
    size_t cap
) {
    (void)head;
    (void)out;
    (void)cap;
    return 0;
}
