#ifndef LIMINAL_EXECUTOR_H
#define LIMINAL_EXECUTOR_H


#include "./memory/memory.h"
#include "./scope/scope.h"
#include "./stack/stack.h"
#include "./step/step.h"
#include "./storage/storage.h"
#include "./universe/universe.h"
#include "./variable/variable.h"
#include "./world/world.h"
#include "../frontends/frontends.h"


/*
 * Executor entry point
 *
 * Consumes an ASTProgram and produces
 * an execution artifact owned by the executor.
 */
Universe *executor_build(const ASTProgram *ast);

/*
 * Dump execution artifact (read-only)
 */
void executor_dump(const Universe *u);

#endif /* LIMINAL_EXECUTOR_H */
