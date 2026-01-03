#include <stdio.h>
#include "./fix_surface.h"
#include "../../executor/step/step.h"

void render_fix_surface(const FixSurface *fs)
{
    printf("\n== Minimal Fix Surface ==\n");

    for (size_t i = 0; i < fs->count; i++) {
        const CauseKey *c = &fs->causes[i];

        printf(
            "FIX: step=%s ast=%llu scope=%llu\n",
            step_kind_str(c->step),
            (unsigned long long)c->ast_id,
            (unsigned long long)c->scope_id
        );
    }
}
