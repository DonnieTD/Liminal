#ifndef LIMINAL_EXECUTOR_H
#define LIMINAL_EXECUTOR_H

#include "frontends/c/ast.h"
#include "executor/universe.h"

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
