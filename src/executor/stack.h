#ifndef LIMINAL_STACK_H
#define LIMINAL_STACK_H

#include <stdint.h>

/*
 * CallStack
 *
 * Represents the active call stack.
 *
 * Stack frames are immutable once created.
 * Calls push frames.
 * Returns restore the previous frame.
 *
 * No execution logic belongs here.
 */

struct Scope;

typedef struct StackFrame {
    /* Lexical scope associated with this frame */
    struct Scope *scope;

    /* Optional metadata (function identity, return site, etc.) */
    void *meta;

    /* Previous frame */
    struct StackFrame *prev;
} StackFrame;

typedef struct CallStack {
    /* Top of the call stack */
    StackFrame *top;

    /* Current depth (optional but useful for analysis) */
    uint64_t depth;
} CallStack;

#endif /* LIMINAL_STACK_H */
