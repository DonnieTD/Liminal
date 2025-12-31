#ifndef ANALYZER_SHADOW_H
#define ANALYZER_SHADOW_H

#include <stddef.h>
#include <stdint.h>

#include "analyzer/diagnostic.h"

struct World;

/*
 * Analyze variable shadowing and redeclaration.
 *
 * Emits diagnostics:
 *  - DIAG_REDECLARATION
 *  - DIAG_SHADOWING
 *
 * Returns number of diagnostics written
 * (or that would be written if out == NULL).
 */
size_t analyze_shadowing(
    struct World *head,
    Diagnostic *out,
    size_t cap
);

#endif /* ANALYZER_SHADOW_H */
