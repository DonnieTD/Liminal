#include "executor/executor.h"

/*
 * Canonical stringification for StepKind
 *
 * This symbol MUST be exported.
 * It is relied upon by renderers and consumers.
 */
const char *step_kind_name(StepKind kind)
{
    switch (kind) {
        case STEP_UNKNOWN:        return "unknown";

        /* Program structure */
        case STEP_ENTER_PROGRAM:  return "enter_program";
        case STEP_EXIT_PROGRAM:   return "exit_program";
        case STEP_ENTER_FUNCTION: return "enter_function";
        case STEP_EXIT_FUNCTION:  return "exit_function";

        /* Control flow */
        case STEP_CALL:           return "call";
        case STEP_RETURN:         return "return";

        /* Scopes */
        case STEP_ENTER_SCOPE:    return "enter_scope";
        case STEP_EXIT_SCOPE:     return "exit_scope";

        /* Variables */
        case STEP_DECLARE:        return "declare";
        case STEP_USE:            return "use";
        case STEP_ASSIGN:         return "assign";

        /* Memory (future) */
        case STEP_LOAD:           return "load";
        case STEP_STORE:          return "store";

        /* Catch-all */
        case STEP_OTHER:          return "other";

        default:                  return "invalid";
    }
}
