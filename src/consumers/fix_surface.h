#pragma once
#include <stddef.h>
#include "consumers/cause_key.h"

typedef struct {
    CauseKey *causes;
    size_t count;
    size_t capacity;
} FixSurface;
