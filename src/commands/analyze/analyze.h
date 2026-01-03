#ifndef LIMINAL_CMD_ANALYZE_H
#define LIMINAL_CMD_ANALYZE_H

#include <stdio.h>
#include <stdlib.h>

#include "../../analyzer/analyzer.h"
#include "../../consumers/consumers.h"




/*
 * cmd_analyze
 *
 * Stage 7:
 *  - derive diagnostics
 *  - derive root chains (ephemeral)
 *  - derive convergence
 *  - derive minimal fix surface
 *
 * NO mutation
 * NO persistence
 * NO cross-stage storage
 */
int cmd_analyze(const struct World *world);

#endif
