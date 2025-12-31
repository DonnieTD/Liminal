#pragma once

#include <stddef.h>

#include "executor/world.h"        // ✅ REQUIRED
#include "analyzer/diagnostic.h"

size_t analyze_use_validation(
    struct World *head,
    Diagnostic *out,
    size_t cap
);
