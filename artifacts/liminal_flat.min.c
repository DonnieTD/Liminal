/* LIMINAL_FLAT_MIN 20251231T164251Z */
//@header src/analyzer/diagnostic.h
#pragma once
#include <stdint.h>
#include <stddef.h>
struct ASTNode;
typedef enum DiagnosticKind {
    DIAG_REDECLARATION,
    DIAG_SHADOWING,
    DIAG_USE_BEFORE_DECLARE,
    DIAG_USE_AFTER_SCOPE_EXIT,
} DiagnosticKind;
typedef struct Diagnostic {
    DiagnosticKind kind;
    uint64_t time;
    uint64_t scope_id;
    uint64_t previous_scope_id;
    const char *name;
    const struct ASTNode *origin;
    const struct ASTNode *previous_origin;
} Diagnostic;
typedef struct DiagnosticArtifact {
    Diagnostic *items;
    size_t count;
} DiagnosticArtifact;
struct World;
DiagnosticArtifact analyze_diagnostics(struct World *head);
void diagnostic_dump(const DiagnosticArtifact *a);
//@header src/analyzer/lifetime.h
#ifndef LIMINAL_LIFETIME_H
#define LIMINAL_LIFETIME_H
#include <stdint.h>
#include <stddef.h>
struct World;
typedef struct ScopeLifetime {
    uint64_t scope_id;
    uint64_t enter_time;
    uint64_t exit_time;   /* UINT64_MAX means "still open" */
    void    *enter_origin;
    void    *exit_origin;
} ScopeLifetime;
/*
 * Collect scope lifetimes into `out`.
 *
 * Returns number of lifetimes written (<= cap).
 * If cap is too small, it truncates (for now).
 */
size_t lifetime_collect_scopes(struct World *head,
                               ScopeLifetime *out,
                               size_t cap);
#endif /* LIMINAL_LIFETIME_H */
//@header src/analyzer/shadow.h
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
//@header src/analyzer/trace.h
#ifndef LIMINAL_TRACE_H
#define LIMINAL_TRACE_H
#include <stdint.h>
struct World;
/*
 * Trace
 *
 * A read-only cursor over a sequence of Worlds.
 *
 * The Trace never mutates Worlds.
 * It may move forward and backward in time.
 */
typedef struct Trace {
    struct World *current;
} Trace;
/* Construction */
Trace trace_begin(struct World *head);
Trace trace_end(struct World *tail);
/* Navigation */
struct World *trace_current(Trace *t);
struct World *trace_next(Trace *t);
struct World *trace_prev(Trace *t);
/* Utility */
int trace_is_valid(const Trace *t);
#endif /* LIMINAL_TRACE_H */
//@header src/analyzer/use.h
#ifndef LIMINAL_ANALYZER_USE_H
#define LIMINAL_ANALYZER_USE_H
#include <stdint.h>
#include <stddef.h>
struct World;
struct ScopeLifetime;
/*
 * Classification of a variable use
 */
typedef enum UseKind {
    USE_OK = 0,
    USE_BEFORE_DECLARE,
    USE_AFTER_SCOPE
} UseKind;
/*
 * Result of analyzing a STEP_USE
 */
typedef struct UseReport {
    uint64_t time;
    uint64_t scope_id;
    uint64_t storage_id; /* UINT64_MAX if unresolved */
    UseKind kind;
} UseReport;
/*
 * Analyze STEP_USE events in a World timeline.
 *
 * - worlds: head of world list
 * - lifetimes: collected scope lifetimes
 * - lifetime_count: number of lifetimes
 * - out: output array
 * - cap: capacity of output array
 *
 * Returns number of reports written.
 */
size_t analyze_step_use(
    const struct World *worlds,
    const struct ScopeLifetime *lifetimes,
    size_t lifetime_count,
    UseReport *out,
    size_t cap
);
#endif /* LIMINAL_ANALYZER_USE_H */
//@header src/analyzer/use_report.h
#ifndef LIMINAL_USE_REPORT_H
#define LIMINAL_USE_REPORT_H
#include <stdint.h>
typedef enum UseKind {
    USE_OK = 0,
    USE_BEFORE_DECLARE,
    USE_AFTER_SCOPE
} UseKind;
typedef struct UseReport {
    uint64_t time;
    uint64_t storage_id;   /* UINT64_MAX if unresolved */
    uint64_t scope_id;
    UseKind  kind;
} UseReport;
#endif
//@header src/analyzer/use_validate.h
#pragma once
#include <stddef.h>
#include "executor/world.h"        // ✅ REQUIRED
#include "analyzer/diagnostic.h"
size_t analyze_use_validation(
    struct World *head,
    Diagnostic *out,
    size_t cap
);
//@header src/analyzer/validate.h
#ifndef LIMINAL_VALIDATE_H
#define LIMINAL_VALIDATE_H
#include <stddef.h>
#include <stdint.h>
/*
 * This file defines *structural validators* over the World timeline.
 *
 * Validators:
 *  - NEVER mutate Worlds
 *  - NEVER execute semantics
 *  - ONLY read derived structure from the trace
 *
 * Their job is to answer questions like:
 *   "Does this execution make sense?"
 *   "Is the structure well-formed?"
 *
 * This is where Liminal first learns to say:
 *   "This program is wrong."
 */
struct World;
/*
 * ScopeViolationKind
 *
 * Enumerates the kinds of structural errors we can detect
 * in scope behavior purely from the trace.
 *
 * These are NOT runtime errors.
 * These are *semantic shape violations*.
 */
typedef enum ScopeViolationKind {
    SCOPE_OK = 0,
    /*
     * An EXIT_SCOPE occurred when no scope was active.
     * This indicates:
     *  - frontend bug
     *  - executor bug
     *  - malformed input
     */
    SCOPE_EXIT_WITHOUT_ENTER,
    /*
     * A scope was entered but never exited.
     * This is detected at end-of-trace.
     */
    SCOPE_ENTER_WITHOUT_EXIT,
    /*
     * Scopes must be exited in LIFO order.
     * If scope A enters, then scope B enters,
     * scope B MUST exit before scope A.
     */
    SCOPE_NON_LIFO_EXIT,
    /*
     * The active_scope pointer stored in the World
     * does not match what the step-derived scope stack
     * says it *should* be.
     *
     * This detects:
     *  - executor bookkeeping bugs
     *  - illegal state transitions
     */
    SCOPE_ACTIVE_MISMATCH
} ScopeViolationKind;
/*
 * ScopeViolation
 *
 * A single detected violation of scope invariants.
 *
 * This is intentionally minimal:
 *  - what went wrong
 *  - when it happened
 *  - which scope was involved
 */
typedef struct ScopeViolation {
    ScopeViolationKind kind;
    /* World time at which violation was observed */
    uint64_t time;
    /* Scope id involved in the violation */
    uint64_t scope_id;
} ScopeViolation;
/*
 * validate_scope_invariants
 *
 * Walks the World timeline from beginning to end
 * and validates that scope ENTER / EXIT events form
 * a well-structured, properly nested tree.
 *
 * Inputs:
 *   - head : first World in timeline
 *   - out  : caller-provided array for violations
 *   - cap  : capacity of `out`
 *
 * Output:
 *   - returns number of violations written
 *
 * Notes:
 *   - Validation is PURE.
 *   - No allocation.
 *   - No mutation.
 *   - If cap is exceeded, results are truncated.
 */
size_t validate_scope_invariants(struct World *head,
                                 ScopeViolation *out,
                                 size_t cap);
#endif /* LIMINAL_VALIDATE_H */
//@header src/analyzer/variable_lifetime.h
#ifndef LIMINAL_VARIABLE_LIFETIME_H
#define LIMINAL_VARIABLE_LIFETIME_H
#include <stdint.h>
#include <stddef.h>
struct World;
typedef struct VariableLifetime {
    uint64_t var_id;
    uint64_t scope_id;
    uint64_t declare_time;
    uint64_t end_time;   /* scope exit */
} VariableLifetime;
size_t lifetime_collect_variables(struct World *head,
                                  VariableLifetime *out,
                                  size_t cap);
#endif
//@header src/common/arena.h
#ifndef LIMINAL_ARENA_H
#define LIMINAL_ARENA_H
#include <stddef.h>
typedef struct Arena {
    unsigned char *base;
    size_t capacity;
    size_t offset;
} Arena;
void arena_init(Arena *a, size_t capacity);
void *arena_alloc(Arena *a, size_t size);
void arena_reset(Arena *a);
void arena_destroy(Arena *a);
#endif
//@header src/common/file.h
#ifndef LIMINAL_FILE_H
#define LIMINAL_FILE_H
#include <stddef.h>
/*
 * Read entire file into heap-allocated buffer.
 *
 * On success:
 *   - *out_buf points to malloc'd memory
 *   - *out_len contains byte length
 *
 * Caller owns the buffer.
 */
int read_entire_file(
    const char *path,
    char      **out_buf,
    size_t     *out_len
);
#endif /* LIMINAL_FILE_H */
//@header src/common/hashmap.h
#ifndef LIMINAL_HASHMAP_H
#define LIMINAL_HASHMAP_H
#include <stddef.h>
struct Arena;
/*
 * HashMap
 *
 * Minimal string-key hashmap.
 * Keys are NOT owned.
 * Values are opaque pointers.
 *
 * Allocation is arena-backed and monotonic.
 */
typedef struct HashMap HashMap;
/* Create an empty hashmap */
HashMap *hashmap_create(struct Arena *arena, size_t bucket_count);
/* Shallow clone (structural copy of buckets) */
HashMap *hashmap_clone(HashMap *src, struct Arena *arena);
/* Insert or overwrite */
void hashmap_put(HashMap *map, const char *key, void *value);
/* Lookup (NULL if missing) */
void *hashmap_get(HashMap *map, const char *key);
#endif /* LIMINAL_HASHMAP_H */
//@header src/common/types.h
//@header src/common/util.h
//@header src/executor/executor.h
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
//@header src/executor/memory.h
#ifndef LIMINAL_MEMORY_H
#define LIMINAL_MEMORY_H
#include <stdint.h>
/*
 * Memory
 *
 * Represents the program's memory model.
 *
 * This is NOT raw bytes.
 * This is semantic storage with identity, lifetime, and aliasing.
 *
 * Memory objects are created, referenced, and destroyed over time.
 * Lifetime is determined by presence in the current World.
 */
struct Arena;
struct HashMap;
typedef struct Memory {
    /* Underlying allocation arenas */
    struct Arena *arena;
    /*
     * Pointer / alias map:
     *   logical address -> memory object
     *
     * Used to track aliasing, lifetimes, and invalid access.
     */
    struct HashMap *objects;
} Memory;
#endif /* LIMINAL_MEMORY_H */
//@header src/executor/scope.h
#include <stdint.h>
#ifndef LIMINAL_SCOPE_H
#define LIMINAL_SCOPE_H
/*
 * Scope
 *
 * A Scope represents a lexical environment.
 *
 * Scopes are immutable once created.
 * New scopes are created by pointing to a parent scope.
 *
 * Scope lookup is purely structural:
 *   - if a name exists in the current scope, it is in scope
 *   - otherwise, lookup proceeds to the parent
 */
struct HashMap;
typedef struct Scope {
     uint64_t id;
    /* Parent lexical scope (NULL for root / file scope) */
    struct Scope *parent;
    /* Bindings for this scope: name -> storage location */
    struct HashMap *bindings;
} Scope;
#endif /* LIMINAL_SCOPE_H */
//@header src/executor/stack.h
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
//@header src/executor/step.h
#ifndef LIMINAL_STEP_H
#define LIMINAL_STEP_H
#include <stdint.h>
/*
 * StepKind
 *
 * Enumerates the semantic reason *why* a World transition occurred.
 *
 * IMPORTANT:
 *  - StepKinds do NOT execute logic
 *  - They describe causality, not behavior
 *  - Analysis code relies on these being stable
 */
typedef enum StepKind {
    STEP_UNKNOWN = 0,
    /* Structural */
    STEP_ENTER_PROGRAM,
    STEP_EXIT_PROGRAM,
    STEP_ENTER_FUNCTION,
    STEP_EXIT_FUNCTION,
    /* Statements */
    STEP_RETURN,
    /* Existing (unused yet) */
    STEP_ENTER_SCOPE,
    STEP_EXIT_SCOPE,
    STEP_CALL,
    STEP_DECLARE,
    STEP_USE,
    STEP_ASSIGN,
    STEP_LOAD,
    STEP_STORE,
    STEP_OTHER
} StepKind;
/*
 * Step
 *
 * A Step represents the semantic *cause* of a World transition.
 *
 * Steps:
 *  - do NOT execute logic
 *  - do NOT own memory
 *  - are immutable once created
 *
 * Interpretation of `info` depends on StepKind:
 *  - ENTER / EXIT_SCOPE → scope id
 *  - DECLARE             → variable id
 *  - others              → kind-specific later
 */
typedef struct Step {
    StepKind kind;
    void    *origin;   // <-- opaque pointer
    uint64_t info;     // <-- kind-specific ID
} Step;
#endif /* LIMINAL_STEP_H */
//@header src/executor/storage.h
#ifndef LIMINAL_STORAGE_H
#define LIMINAL_STORAGE_H
#include <stdint.h>
/*
 * Storage
 *
 * Represents a concrete storage location created by a declaration.
 * Storage has identity and lifetime, but no value yet.
 */
typedef struct Storage {
    uint64_t id;
    uint64_t declared_at;
} Storage;
#endif /* LIMINAL_STORAGE_H */
//@header src/executor/universe.h
#ifndef LIMINAL_UNIVERSE_H
#define LIMINAL_UNIVERSE_H
#include <stdint.h>
#include "executor/world.h"
#include "common/arena.h"
/*
 * Universe
 *
 * The Universe owns time and history.
 *
 * It is responsible for:
 * - creating Worlds
 * - linking Worlds into a timeline
 * - tracking the current World
 *
 * The Universe does NOT:
 * - execute semantics
 * - analyze Worlds
 * - mutate existing Worlds
 */
typedef struct Universe {
    uint64_t current_time;
    World *head;
    World *tail;
    World *current;
    Arena world_arena;
    Arena step_arena;
    Arena scope_arena;
    Arena var_arena;
    Arena storage_arena;
    /* Identity counters */
    uint64_t next_scope_id;
    uint64_t next_var_id;
    uint64_t next_storage_id;
} Universe;
World *universe_step(Universe *u, void *origin);
void universe_attach_initial_world(Universe *u, World *w);
Universe *universe_create(void);
/* Scope control */
World *universe_enter_scope(Universe *u, void *origin);
World *universe_exit_scope(Universe *u, void *origin);
/* Variable operations */
World *universe_declare_variable(
    Universe *u,
    const char *name,
    void *origin
);
World *universe_use_variable(
    Universe *u,
    const char *name,
    void *origin
);
#endif /* LIMINAL_UNIVERSE_H */
//@header src/executor/variable.h
#ifndef LIMINAL_VARIABLE_H
#define LIMINAL_VARIABLE_H
#include <stdint.h>
/*
 * Variable
 *
 * A Variable represents a named storage identity.
 * It exists within a specific lexical scope.
 *
 * Variables are immutable.
 */
typedef struct Variable {
    uint64_t id;          /* unique id */
    uint64_t scope_id;    /* scope that owns this variable */
    const char *name;     /* optional (frontend later) */
} Variable;
#endif /* LIMINAL_VARIABLE_H */
//@header src/executor/world.h
#ifndef LIMINAL_WORLD_H
#define LIMINAL_WORLD_H
#include <stdint.h>
/*
 * World
 *
 * A World represents the complete semantic state of the program
 * at a single moment in time.
 */
struct Universe;   /* forward declaration */
struct Scope;
struct CallStack;
struct Memory;
struct Step;
typedef struct World {
    uint64_t time;
    struct Scope     *active_scope;
    struct CallStack *call_stack;
    struct Memory    *memory;
    struct Step      *step;
    struct World     *prev;
    struct World     *next;
} World;
/* Constructors */
World *world_create_initial(struct Universe *u);
/* Cloning */
World *world_clone(struct Universe *u, const World *src);
#endif /* LIMINAL_WORLD_H */
//@header src/frontends/c/ast.h
#ifndef LIMINAL_C_AST_H
#define LIMINAL_C_AST_H
#include <stdint.h>
#include <stddef.h>
typedef enum ASTKind {
    AST_UNKNOWN = 0,
    /* Top-level */
    AST_PROGRAM,
    AST_FUNCTION,
    /* Structure */
    AST_BLOCK,
    AST_STATEMENT,
    AST_CALL,
    /* Statements */
    AST_VAR_DECL,   /* e.g. int x; */
    AST_VAR_USE,    /* e.g. x; (as a statement for now) */
    AST_RETURN      /* e.g. return 0; */
} ASTKind;
typedef struct ASTSpan {
    uint32_t line;
    uint32_t col;
} ASTSpan;
typedef struct ASTNode ASTNode;
typedef struct ASTProgram {
    /* Arena-owned nodes */
    ASTNode  *nodes;
    size_t    count;
    size_t    cap;
    /* Source (owned copy) */
    char     *source_path;
    char     *source_text;
    size_t    source_len;
    /* Root node id (index+1) */
    uint32_t  root_id;
} ASTProgram;
/* Node payloads (keep these dead simple for Step 1) */
typedef struct ASTFunction {
    const char *name;   /* owned by ASTProgram */
    uint32_t    body_id;
} ASTFunction;
typedef struct ASTBlock {
    uint32_t *stmt_ids;   /* array of node ids */
    size_t    stmt_count;
} ASTBlock;
typedef struct ASTVarDecl {
    const char *name;     /* points into program-owned memory */
} ASTVarDecl;
typedef struct ASTVarUse {
    const char *name;     /* points into program-owned memory */
} ASTVarUse;
typedef struct ASTReturn {
    int64_t value;        /* only integer literals for now */
} ASTReturn;
struct ASTNode {
    uint32_t id;          /* stable id: index+1 */
    ASTKind  kind;
    ASTSpan  at;
    union {
        ASTFunction fn;
        ASTBlock    block;
        ASTVarDecl  vdecl;
        ASTVarUse   vuse;
        ASTReturn   ret;
    } as;
};
/* Artifact API */
ASTProgram *ast_program_new(const char *source_path, const char *source_text, size_t len);
void        ast_program_free(ASTProgram *p);
ASTNode    *ast_node_get(ASTProgram *p, uint32_t id);
/* Builders */
uint32_t    ast_add_node(ASTProgram *p, ASTKind kind, ASTSpan at);
/* Debug */
void        ast_dump(const ASTProgram *p);
#endif /* LIMINAL_C_AST_H */
//@header src/frontends/c/frontend.h
#ifndef LIMINAL_FRONTEND_C_H
#define LIMINAL_FRONTEND_C_H
#include "frontends/c/ast.h"
/*
 * Frontend artifact:
 *
 * Parse a C source file into an immutable ASTProgram.
 *
 * Ownership:
 *   - Returned ASTProgram is heap / arena owned
 *   - Caller is responsible for freeing it
 */
ASTProgram *c_parse_file_to_ast(const char *path);
#endif /* LIMINAL_FRONTEND_C_H */
//@header src/frontends/c/lexer.h
#ifndef LIMINAL_C_LEXER_H
#define LIMINAL_C_LEXER_H
#include <stddef.h>
typedef enum TokKind {
    TOK_EOF = 0,
    /* Keywords */
    TOK_INT,
    TOK_RETURN,
    /* Identifiers / literals */
    TOK_IDENT,
    TOK_INT_LIT,
    /* Punctuation */
    TOK_LPAREN,   /* ( */
    TOK_RPAREN,   /* ) */
    TOK_LBRACE,   /* { */
    TOK_RBRACE,   /* } */
    TOK_SEMI      /* ; */
} TokKind;
typedef struct Token {
    TokKind kind;
    const char *lexeme;   /* points into source */
    size_t len;
} Token;
typedef struct Lexer {
    const char *path;
    const char *src;
    size_t      len;
    size_t      pos;
} Lexer;
/* API */
void  lexer_init(Lexer *lx, const char *path, const char *src, size_t len);
Token lexer_next(Lexer *lx);
int   lexer_accept(Lexer *lx, TokKind k);
#endif /* LIMINAL_C_LEXER_H */
//@header src/frontends/c/parser.h
#ifndef LIMINAL_C_PARSER_H
#define LIMINAL_C_PARSER_H
#include "frontends/c/ast.h"
#include "frontends/c/lexer.h"
/*
 * Parser
 *
 * Stage-1 parser stub.
 *
 * Responsibility:
 *   - consume a lexer
 *   - produce an ASTProgram artifact
 *
 * No real grammar yet.
 */
ASTProgram *parse_translation_unit(Lexer *lx);
#endif /* LIMINAL_C_PARSER_H */
//@source src/analyzer/diagnostic.c
#include "analyzer/diagnostic.h"
#include "analyzer/shadow.h"
#include "analyzer/use_validate.h"
#include <stdlib.h>
DiagnosticArtifact analyze_diagnostics(struct World *head)
{
    size_t cap = 128;
    Diagnostic *buf = calloc(cap, sizeof(Diagnostic));
    size_t count = 0;
    count += analyze_shadowing(head, buf + count, cap - count);
    count += analyze_use_validation(head, buf + count, cap - count);
    return (DiagnosticArtifact){
        .items = buf,
        .count = count
    };
}
//@source src/analyzer/diagnostic_dump.c
#include "analyzer/diagnostic.h"
#include <stdio.h>
static const char *kind_str(DiagnosticKind k)
{
    switch (k) {
    case DIAG_REDECLARATION: return "REDECLARATION";
    case DIAG_SHADOWING: return "SHADOWING";
    case DIAG_USE_BEFORE_DECLARE: return "USE_BEFORE_DECLARE";
    case DIAG_USE_AFTER_SCOPE_EXIT: return "USE_AFTER_SCOPE_EXIT";
    default: return "UNKNOWN";
    }
}
void diagnostic_dump(const DiagnosticArtifact *a)
{
    if (a->count == 0)
        return;
    printf("\n-- DIAGNOSTICS --\n");
    for (size_t i = 0; i < a->count; i++) {
        const Diagnostic *d = &a->items[i];
        printf(
            "time=%llu %s name=%s scope=%llu prev_scope=%llu\n",
            (unsigned long long)d->time,
            kind_str(d->kind),
            d->name ? d->name : "?",
            (unsigned long long)d->scope_id,
            (unsigned long long)d->previous_scope_id
        );
    }
}
//@source src/analyzer/lifetime.c
#include <stdint.h>
#include <stddef.h>
#include "analyzer/lifetime.h"
#include "analyzer/trace.h"
#include "executor/world.h"
#include "executor/step.h"
/*
 * We derive scope lifetimes purely from Steps:
 *  - STEP_ENTER_SCOPE: open lifetime
 *  - STEP_EXIT_SCOPE : close lifetime
 *
 * Step->info carries the scope id for both enter and exit.
 */
size_t lifetime_collect_scopes(struct World *head,
                               ScopeLifetime *out,
                               size_t cap)
{
    if (!head || !out || cap == 0) {
        return 0;
    }
    size_t n = 0;
    Trace t = trace_begin(head);
    while (trace_is_valid(&t)) {
        World *w = trace_current(&t);
        if (!w || !w->step) {
            trace_next(&t);
            continue;
        }
        Step *s = w->step;
        if (s->kind == STEP_ENTER_SCOPE) {
            if (n >= cap) {
                return n; /* truncate for now */
            }
            out[n].scope_id      = s->info;
            out[n].enter_time    = w->time;
            out[n].exit_time     = UINT64_MAX;
            out[n].enter_origin  = s->origin;
            out[n].exit_origin   = NULL;
            n++;
        } else if (s->kind == STEP_EXIT_SCOPE) {
            /* close the most recent open lifetime with matching scope_id */
            uint64_t sid = s->info;
            for (size_t i = n; i > 0; i--) {
                ScopeLifetime *lt = &out[i - 1];
                if (lt->scope_id == sid && lt->exit_time == UINT64_MAX) {
                    lt->exit_time   = w->time;
                    lt->exit_origin = s->origin;
                    break;
                }
            }
        }
        trace_next(&t);
    }
    return n;
}
//@source src/analyzer/shadow.c
#include "analyzer/shadow.h"
#include "executor/world.h"
#include "executor/step.h"
#include "frontends/c/ast.h"
#include "common/hashmap.h"
#include "common/arena.h"
#include "analyzer/diagnostic.h"
#include <stdlib.h>
#define MAX_SCOPE_DEPTH 64
#define SHADOW_BUCKETS 32
/* ---------------------------------------------------------
 * Analyzer-owned arena
 *
 * Shadow analysis memory is process-lifetime.
 * This arena is never reset or destroyed (by design).
 * --------------------------------------------------------- */
static Arena shadow_arena;
static int shadow_arena_initialized = 0;
static void ensure_shadow_arena(void)
{
    if (!shadow_arena_initialized) {
        arena_init(&shadow_arena, 64 * 1024);
        shadow_arena_initialized = 1;
    }
}
/* ---------------------------------------------------------
 * Per-scope declaration record (internal only)
 * --------------------------------------------------------- */
typedef struct ShadowDecl {
    const char    *name;
    uint64_t       scope_id;
    uint64_t       time;
    const ASTNode *origin;
} ShadowDecl;
typedef struct ShadowFrame {
    uint64_t scope_id;
    HashMap *decls; /* name -> ShadowDecl* */
} ShadowFrame;
/* --------------------------------------------------------- */
static ShadowFrame shadow_frame_new(uint64_t scope_id)
{
    ensure_shadow_arena();
    ShadowFrame f;
    f.scope_id = scope_id;
    f.decls = hashmap_create(&shadow_arena, SHADOW_BUCKETS);
    return f;
}
/* No-op by design: arena-backed, process-lifetime */
static void shadow_frame_free(ShadowFrame *f)
{
    (void)f;
}
/* ---------------------------------------------------------
 * Extract variable name from STEP_DECLARE origin
 * --------------------------------------------------------- */
static const char *decl_name_from_step(const Step *s)
{
    if (!s || !s->origin)
        return NULL;
    const ASTNode *n = (const ASTNode *)s->origin;
    if (n->kind != AST_VAR_DECL)
        return NULL;
    return n->as.vdecl.name;
}
/* --------------------------------------------------------- */
size_t analyze_shadowing(
    struct World *head,
    Diagnostic *out,
    size_t cap
) {
    ShadowFrame stack[MAX_SCOPE_DEPTH];
    size_t depth = 0;
    size_t count = 0;
    for (World *w = head; w; w = w->next) {
        Step *s = w->step;
        if (!s)
            continue;
        switch (s->kind) {
        case STEP_ENTER_SCOPE:
            if (depth < MAX_SCOPE_DEPTH) {
                stack[depth++] = shadow_frame_new(s->info);
            }
            break;
        case STEP_EXIT_SCOPE:
            if (depth > 0) {
                shadow_frame_free(&stack[--depth]);
            }
            break;
        case STEP_DECLARE: {
            if (depth == 0)
                break;
            const char *name = decl_name_from_step(s);
            if (!name)
                break;
            ShadowFrame *cur = &stack[depth - 1];
            /* 1. Redeclaration in same scope */
            ShadowDecl *existing = hashmap_get(cur->decls, name);
            if (existing) {
                if (out && count < cap) {
                    out[count] = (Diagnostic){
                        .kind = DIAG_REDECLARATION,
                        .time = w->time,
                        .scope_id = cur->scope_id,
                        .previous_scope_id = cur->scope_id,
                        .name = name,
                        .origin = s->origin,
                        .previous_origin = existing->origin
                    };
                }
                count++;
                break;
            }
            /* 2. Shadowing outer scopes */
            for (size_t i = depth - 1; i-- > 0;) {
                ShadowFrame *parent = &stack[i];
                ShadowDecl *outer = hashmap_get(parent->decls, name);
                if (outer) {
                    if (out && count < cap) {
                        out[count] = (Diagnostic){
                            .kind = DIAG_SHADOWING,
                            .time = w->time,
                            .scope_id = cur->scope_id,
                            .previous_scope_id = parent->scope_id,
                            .name = name,
                            .origin = s->origin,
                            .previous_origin = outer->origin
                        };
                    }
                    count++;
                    break;
                }
            }
            /* Record declaration */
            ShadowDecl *decl =
                arena_alloc(&shadow_arena, sizeof(ShadowDecl));
            if (!decl)
                break;
            *decl = (ShadowDecl){
                .name = name,
                .scope_id = cur->scope_id,
                .time = w->time,
                .origin = s->origin
            };
            hashmap_put(cur->decls, name, decl);
            break;
        }
        default:
            break;
        }
    }
    return count;
}
//@source src/analyzer/trace.c
#include "analyzer/trace.h"
#include "executor/world.h"
#include <stdlib.h>
/*
 * Create a Trace starting at the beginning of time.
 */
Trace trace_begin(struct World *head)
{
    Trace t;
    t.current = head;
    return t;
}
/*
 * Create a Trace starting at the end of time.
 */
Trace trace_end(struct World *tail)
{
    Trace t;
    t.current = tail;
    return t;
}
/*
 * Get the current World.
 */
struct World *trace_current(Trace *t)
{
    if (!t) {
        return NULL;
    }
    return t->current;
}
/*
 * Advance the Trace forward in time.
 */
struct World *trace_next(Trace *t)
{
    if (!t || !t->current) {
        return NULL;
    }
    t->current = t->current->next;
    return t->current;
}
/*
 * Move the Trace backward in time.
 */
struct World *trace_prev(Trace *t)
{
    if (!t || !t->current) {
        return NULL;
    }
    t->current = t->current->prev;
    return t->current;
}
/*
 * Check whether the Trace points to a valid World.
 */
int trace_is_valid(const Trace *t)
{
    return t && t->current;
}
//@source src/analyzer/use.c
#include "analyzer/use.h"
#include "analyzer/trace.h"
#include "executor/world.h"
#include "executor/step.h"
#include "lifetime.h"
#include "executor/scope.h"
size_t analyze_step_use(
    const struct World *worlds,
    const struct ScopeLifetime *lifetimes,
    size_t lifetime_count,
    UseReport *out,
    size_t cap
) {
    size_t count = 0;
    Trace t = trace_begin((struct World *)worlds);
    while (trace_is_valid(&t)) {
        const World *w = trace_current(&t);
        const Step *s = w->step;
        if (!s || s->kind != STEP_USE)
            goto next;
        if (count >= cap)
            break;
        UseReport r = {
            .time = w->time,
            .scope_id = w->active_scope ? w->active_scope->id : 0,
            .storage_id = s->info,
            .kind = USE_OK
        };
        /* Rule 1: use before declaration */
        if (s->info == UINT64_MAX) {
            r.kind = USE_BEFORE_DECLARE;
            out[count++] = r;
            goto next;
        }
        /* Rule 2: use after scope exit */
        for (size_t i = 0; i < lifetime_count; i++) {
            const ScopeLifetime *lt = &lifetimes[i];
            if (lt->scope_id == r.scope_id &&
                lt->exit_time != UINT64_MAX &&
                r.time > lt->exit_time) {
                r.kind = USE_AFTER_SCOPE;
                out[count++] = r;
                goto next;
            }
        }
    next:
        trace_next(&t);
    }
    return count;
}
//@source src/analyzer/use_validate.c
#include "analyzer/use_validate.h"
#include "analyzer/use.h"
#include "analyzer/lifetime.h"
#include "executor/world.h"
size_t analyze_use_validation(
    struct World *head,
    Diagnostic *out,
    size_t cap
) {
    ScopeLifetime lifetimes[128];
    size_t lt_count = lifetime_collect_scopes(head, lifetimes, 128);
    UseReport uses[128];
    size_t use_count = analyze_step_use(
        head,
        lifetimes,
        lt_count,
        uses,
        128
    );
    size_t count = 0;
    for (size_t i = 0; i < use_count && count < cap; i++) {
        const UseReport *u = &uses[i];
        if (u->kind == USE_OK)
            continue;
        out[count++] = (Diagnostic){
            .kind =
                (u->kind == USE_BEFORE_DECLARE)
                    ? DIAG_USE_BEFORE_DECLARE
                    : DIAG_USE_AFTER_SCOPE_EXIT,
            .time = u->time,
            .scope_id = u->scope_id,
            .previous_scope_id = 0,
            .name = NULL,
            .origin = NULL,
            .previous_origin = NULL
        };
    }
    return count;
}
//@source src/analyzer/validate.c
#include "analyzer/validate.h"
#include "analyzer/trace.h"
#include "executor/world.h"
#include "executor/step.h"
#define MAX_SCOPE_DEPTH 128
size_t validate_scope_invariants(
    struct World *head,
    ScopeViolation *out,
    size_t cap
) {
    uint64_t stack[MAX_SCOPE_DEPTH];
    size_t depth = 0;
    size_t count = 0;
    Trace t = trace_begin(head);
    while (trace_is_valid(&t)) {
        World *w = trace_current(&t);
        if (!w || !w->step) {
            trace_next(&t);
            continue;
        }
        Step *s = w->step;
        if (s->kind == STEP_ENTER_SCOPE) {
            if (depth < MAX_SCOPE_DEPTH) {
                stack[depth++] = s->info;
            }
        }
        else if (s->kind == STEP_EXIT_SCOPE) {
            if (depth == 0) {
                if (count < cap) {
                    out[count++] = (ScopeViolation){
                        .kind = SCOPE_EXIT_WITHOUT_ENTER,
                        .time = w->time,
                        .scope_id = s->info
                    };
                }
            } else {
                uint64_t expected = stack[depth - 1];
                if (expected != s->info) {
                    if (count < cap) {
                        out[count++] = (ScopeViolation){
                            .kind = SCOPE_NON_LIFO_EXIT,
                            .time = w->time,
                            .scope_id = s->info
                        };
                    }
                } else {
                    depth--;
                }
            }
        }
        trace_next(&t);
    }
    /* Unclosed scopes */
    for (size_t i = 0; i < depth && count < cap; i++) {
        out[count++] = (ScopeViolation){
            .kind = SCOPE_ENTER_WITHOUT_EXIT,
            .time = UINT64_MAX,
            .scope_id = stack[i]
        };
    }
    return count;
}
//@source src/analyzer/variable_lifetime.c
#include "analyzer/variable_lifetime.h"
#include "analyzer/trace.h"
#include "executor/world.h"
#include "executor/step.h"
#include "executor/scope.h"
size_t lifetime_collect_variables(
    struct World *head,
    VariableLifetime *out,
    size_t cap
) {
    size_t n = 0;
    Trace t = trace_begin(head);
    while (trace_is_valid(&t)) {
        World *w = trace_current(&t);
        Step *s = w->step;
        if (s && s->kind == STEP_DECLARE) {
            if (n >= cap) break;
            out[n++] = (VariableLifetime){
                .var_id        = s->info,
                .scope_id      = w->active_scope ? w->active_scope->id : 0,
                .declare_time  = w->time,
                .end_time      = UINT64_MAX
            };
        }
        if (s && s->kind == STEP_EXIT_SCOPE) {
            uint64_t sid = s->info;
            for (size_t i = 0; i < n; i++) {
                if (out[i].scope_id == sid &&
                    out[i].end_time == UINT64_MAX) {
                    out[i].end_time = w->time;
                }
            }
        }
        trace_next(&t);
    }
    return n;
}
//@source src/common/arena.c
#include <stdlib.h>
#include <string.h>
#include "common/arena.h"
void arena_init(Arena *a, size_t capacity)
{
    a->base = malloc(capacity);
    a->capacity = capacity;
    a->offset = 0;
}
void *arena_alloc(Arena *a, size_t size)
{
    /* align to 8 bytes */
    size = (size + 7) & ~7;
    if (a->offset + size > a->capacity) {
        return NULL;
    }
    void *ptr = a->base + a->offset;
    a->offset += size;
    memset(ptr, 0, size);
    return ptr;
}
void arena_reset(Arena *a)
{
    a->offset = 0;
}
void arena_destroy(Arena *a)
{
    free(a->base);
    a->base = NULL;
    a->capacity = 0;
    a->offset = 0;
}
//@source src/common/file.c
#include "common/file.h"
#include <stdio.h>
#include <stdlib.h>
int read_entire_file(
    const char *path,
    char      **out_buf,
    size_t     *out_len
)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return 0;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return 0;
    }
    size_t n = fread(buf, 1, (size_t)size, f);
    fclose(f);
    if (n != (size_t)size) {
        free(buf);
        return 0;
    }
    buf[size] = '\0';
    *out_buf = buf;
    *out_len = (size_t)size;
    return 1;
}
//@source src/common/hashmap.c
#include "common/hashmap.h"
#include "common/arena.h"
#include <string.h>
#include <stdint.h>
typedef struct HashEntry {
    const char *key;
    void *value;
    struct HashEntry *next;
} HashEntry;
struct HashMap {
    struct Arena *arena;
    size_t bucket_count;
    HashEntry **buckets;
};
/* Simple FNV-1a hash */
static uint64_t hash_str(const char *s)
{
    uint64_t h = 1469598103934665603ULL;
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 1099511628211ULL;
    }
    return h;
}
HashMap *hashmap_create(struct Arena *arena, size_t bucket_count)
{
    if (!arena || bucket_count == 0) {
        return NULL;
    }
    HashMap *map = arena_alloc(arena, sizeof(HashMap));
    if (!map) {
        return NULL;
    }
    map->arena = arena;
    map->bucket_count = bucket_count;
    map->buckets = arena_alloc(arena, sizeof(HashEntry *) * bucket_count);
    if (!map->buckets) {
        return NULL;
    }
    memset(map->buckets, 0, sizeof(HashEntry *) * bucket_count);
    return map;
}
HashMap *hashmap_clone(HashMap *src, struct Arena *arena)
{
    if (!arena) {
        return NULL;
    }
    if (!src) {
        /* Create empty map if no parent */
        return hashmap_create(arena, 32);
    }
    HashMap *map = arena_alloc(arena, sizeof(HashMap));
    if (!map) {
        return NULL;
    }
    map->arena = arena;
    map->bucket_count = src->bucket_count;
    map->buckets = arena_alloc(arena, sizeof(HashEntry *) * map->bucket_count);
    if (!map->buckets) {
        return NULL;
    }
    /* Structural (shallow) copy */
    memcpy(
        map->buckets,
        src->buckets,
        sizeof(HashEntry *) * map->bucket_count
    );
    return map;
}
void hashmap_put(HashMap *map, const char *key, void *value)
{
    if (!map || !key) {
        return;
    }
    uint64_t h = hash_str(key);
    size_t idx = h % map->bucket_count;
    /* Overwrite if exists */
    for (HashEntry *e = map->buckets[idx]; e; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            e->value = value;
            return;
        }
    }
    /* Insert new entry */
    HashEntry *e = arena_alloc(map->arena, sizeof(HashEntry));
    if (!e) {
        return;
    }
    e->key = key;
    e->value = value;
    e->next = map->buckets[idx];
    map->buckets[idx] = e;
}
void *hashmap_get(HashMap *map, const char *key)
{
    if (!map || !key) {
        return NULL;
    }
    uint64_t h = hash_str(key);
    size_t idx = h % map->bucket_count;
    for (HashEntry *e = map->buckets[idx]; e; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            return e->value;
        }
    }
    return NULL;
}
//@source src/executor/executor.c
#include <stdio.h>
#include "executor/executor.h"
#include "executor/world.h"
#include "executor/step.h"
#include "frontends/c/ast.h"
/* Forward */
static void exec_node(Universe *u,
                      const ASTProgram *p,
                      uint32_t node_id);
/* Entry point */
Universe *executor_build(const ASTProgram *p)
{
    if (!p || p->root_id == 0)
        return NULL;
    Universe *u = universe_create();
    if (!u)
        return NULL;
    World *w0 = world_create_initial(u);
    universe_attach_initial_world(u, w0);
    exec_node(u, p, p->root_id);
    return u;
}
/* Recursive structural traversal */
static void exec_node(Universe *u,
                      const ASTProgram *p,
                      uint32_t node_id)
{
    ASTNode *n = ast_node_get((ASTProgram *)p, node_id);
    if (!n) return;
    switch (n->kind) {
    case AST_PROGRAM:
        u->current = universe_step(u, n);
        u->current->step->kind = STEP_ENTER_PROGRAM;
        /* Assume single function for now */
        for (size_t i = 0; i < p->count; i++) {
            if (p->nodes[i].kind == AST_FUNCTION) {
                exec_node(u, p, p->nodes[i].id);
            }
        }
        u->current = universe_step(u, n);
        u->current->step->kind = STEP_EXIT_PROGRAM;
        break;
    case AST_FUNCTION:
        /* Structural marker */
        universe_step(u, n)->step->kind = STEP_ENTER_FUNCTION;
        /* Function introduces a scope */
        universe_enter_scope(u, n);
        /* Execute function body */
        exec_node(u, p, n->as.fn.body_id);
        /* Exit function scope */
        universe_exit_scope(u, n);
        /* Structural marker */
        universe_step(u, n)->step->kind = STEP_EXIT_FUNCTION;
        break;
    case AST_BLOCK:
        /* ENTER_SCOPE */
        universe_enter_scope(u, n);
        /* Execute statements */
        for (size_t i = 0; i < n->as.block.stmt_count; i++) {
            exec_node(u, p, n->as.block.stmt_ids[i]);
        }
        /* EXIT_SCOPE */
        universe_exit_scope(u, n);
        break;
    case AST_RETURN:
        u->current = universe_step(u, n);
        u->current->step->kind = STEP_RETURN;
        break;
    case AST_VAR_DECL:
        universe_declare_variable(
            u,
            n->as.vdecl.name,
            n
        );
        break;
    case AST_VAR_USE:
        universe_use_variable(
            u,
            n->as.vuse.name,
            n
        );
        break;
    default:
        /* Ignore unsupported nodes */
        break;
    }
}
/*
    Dump execution artifact (read-only)
    By convention, WORLD[1] is the initial world.
    How:
        1. Iterate worlds in order
        2. Print step info
        3. Print relevant metadata
        4. Done
*/
void executor_dump(const Universe *u)
{
    if (!u || !u->head) {
        printf("\n-- EXECUTION ARTIFACT --\n(empty)\n");
        return;
    }
    printf("\n-- EXECUTION ARTIFACT --\n");
    printf("world_count=%llu\n\n",
           (unsigned long long)u->current_time + 1);
    printf("WORLD[1]\n");
    for (World *w = u->head; w; w = w->next) {
        Step *s = w->step;
        if (!s) continue;
        printf("  STEP[%llu] ",
               (unsigned long long)w->time);
        switch (s->kind) {
        case STEP_ENTER_PROGRAM:  printf("ENTER_PROGRAM");  break;
        case STEP_EXIT_PROGRAM:   printf("EXIT_PROGRAM");   break;
        case STEP_ENTER_FUNCTION: printf("ENTER_FUNCTION"); break;
        case STEP_EXIT_FUNCTION:  printf("EXIT_FUNCTION");  break;
        case STEP_ENTER_SCOPE:    printf("ENTER_SCOPE");    break;
        case STEP_EXIT_SCOPE:     printf("EXIT_SCOPE");     break;
        case STEP_RETURN:         printf("RETURN");         break;
        case STEP_DECLARE:        printf("DECLARE");        break;
        case STEP_USE:            printf("USE");            break;
        default:                  printf("UNKNOWN");        break;
        }
        if (s->origin) {
            ASTNode *n = (ASTNode *)s->origin;
            printf(" ast=%u", n->id);
        }
        if (s->kind == STEP_DECLARE || s->kind == STEP_USE) {
            printf(" storage=%llu",
                   (unsigned long long)s->info);
        }
        printf("\n");
    }
}
//@source src/executor/memory.c
/* intentionally empty */
void __liminal_stub(void) {}
//@source src/executor/scope.c
/* intentionally empty */
void __liminal_stub2(void) {}
//@source src/executor/stack.c
/* intentionally empty */
void __liminal_stub3(void) {}
//@source src/executor/step.c
/* intentionally empty */
void __liminal_stub4(void) {}
//@source src/executor/universe.c
#include <stdlib.h>
#include "executor/universe.h"
#include "executor/world.h"
#include "executor/scope.h"
#include "executor/step.h"
#include "executor/variable.h"
#include "executor/storage.h"
#include "common/hashmap.h"
/*
 * Create an empty Universe.
 *
 * The Universe owns time and the World chain.
 * Initially, there are no Worlds.
 */
Universe *universe_create(void)
{
    Universe *u = calloc(1, sizeof(Universe));
    if (!u) {
        return NULL;
    }
    u->current_time = 0;
    u->head = NULL;
    u->tail = NULL;
    u->current = NULL;
    arena_init(&u->world_arena, 64 * 1024); /* Worlds */
    arena_init(&u->step_arena, 64 * 1024);  /* Steps */ /* plenty for now */
    arena_init(&u->scope_arena, 64 * 1024); /* Scopes */ 
    arena_init(&u->storage_arena, 64 * 1024); /* Storage */ 
    u->next_scope_id   = 1;
    u->next_storage_id = 1;
    return u;
}
/*
 * Advance the Universe by one step in time.
 *
 * This clones the current World, increments time,
 * links history, and advances the current pointer.
 *
 * No execution happens here.
 * Only causality and time.
 */
World *universe_step(Universe *u, void *origin)
{
    if (!u || !u->current) {
        return NULL;
    }
    World *prev = u->current;
    World *next = world_clone(u, prev);
    if (!next) {
        return NULL;
    }
    /* Advance time */
    next->time = prev->time + 1;
    /* Attach semantic cause with AST origin */
    Step *s = arena_alloc(&u->step_arena, sizeof(Step));
    if (!s) {
        return NULL;
    }
    s->kind   = STEP_OTHER;
    s->origin = origin;
    s->info   = 0;
    next->step = s;
    /* Link temporal history */
    next->prev = prev;
    prev->next = next;
    /* Update Universe */
    u->current = next;
    u->tail = next;
    u->current_time = next->time;
    return next;
}
/*
 * Attach the initial World to the Universe.
 *
 * This establishes time = 0.
 */
 void universe_attach_initial_world(Universe *u, World *w)
{
    if (!u || !w) {
        return;
    }
    /* Allocate initial Step from the Universe arena */
    Step *s = arena_alloc(&u->step_arena, sizeof(Step));
    if (!s) {
        return; /* fatal in practice, but keep function total */
    }
    s->kind   = STEP_UNKNOWN;
    s->origin = NULL;
    s->info   = 0;
    /* Attach causal root */
    w->step = s;
    /* Establish timeline */
    u->head = w;
    u->tail = w;
    u->current = w;
    u->current_time = w->time;
}
/*
 * Enter a new lexical scope.
 *
 * This creates a new Scope, clones the current World,
 * updates the active scope, and links history.
 */
World *universe_enter_scope(Universe *u, void *origin)
{
    if (!u || !u->current) {
        return NULL;
    }
    /* Allocate new Scope */
    Scope *scope = arena_alloc(&u->scope_arena, sizeof(Scope));
    if (!scope) {
        return NULL;
    }
    scope->id       = u->next_scope_id++;
    scope->parent   = u->current->active_scope;
    scope->bindings = NULL; /* later */
    /* Clone world */
    World *next = world_clone(u, u->current);
    if (!next) {
        return NULL;
    }
    next->time = u->current->time + 1;
    next->active_scope = scope;
    /* Create Step */
    Step *s = arena_alloc(&u->step_arena, sizeof(Step));
    if (!s) {
        return NULL;
    }
    s->kind   = STEP_ENTER_SCOPE;
    s->origin = origin;
    s->info   = scope->id;
    next->step = s;
    /* Link timeline */
    next->prev = u->current;
    u->current->next = next;
    u->current = next;
    u->tail = next;
    u->current_time = next->time;
    return next;
}
/*
 * Exit the current lexical scope.
 *
 * This clones the current World, updates the active scope
 * to the parent scope, and links history.
 */
World *universe_exit_scope(Universe *u, void *origin)
{
    if (!u || !u->current || !u->current->active_scope) {
        return NULL;
    }
    Scope *exiting = u->current->active_scope;
    Scope *parent  = exiting->parent;
    /* Clone world */
    World *next = world_clone(u, u->current);
    if (!next) {
        return NULL;
    }
    next->time = u->current->time + 1;
    next->active_scope = parent;
    /* Create Step */
    Step *s = arena_alloc(&u->step_arena, sizeof(Step));
    if (!s) {
        return NULL;
    }
    s->kind   = STEP_EXIT_SCOPE;
    s->origin = origin;
    s->info   = exiting->id;
    next->step = s;
    /* Link timeline */
    next->prev = u->current;
    u->current->next = next;
    u->current = next;
    u->tail = next;
    u->current_time = next->time;
    return next;
}
/*
 * Declare a new variable in the current scope.
 *
 * This creates a new Storage, clones the current World,
 * updates the active scope's bindings, and links history.
 */
World *universe_declare_variable(
    Universe *u,
    const char *name,
    void *origin
)
{
    if (!u || !u->current || !name) {
        return NULL;
    }
    World *prev = u->current;
    World *next = world_clone(u, prev);
    if (!next) {
        return NULL;
    }
    /* Advance time */
    next->time = prev->time + 1;
    /* Allocate Storage */
    Storage *st = arena_alloc(&u->storage_arena, sizeof(Storage));
    if (!st) {
        return NULL;
    }
    st->id = u->next_storage_id++;
    st->declared_at = next->time;
    /* Create new scope frame */
    Scope *old = prev->active_scope;
    Scope *sc = arena_alloc(&u->scope_arena, sizeof(Scope));
    if (!sc) {
        return NULL;
    }
    sc->id = old->id;
    sc->parent = old;
    sc->bindings = hashmap_clone(
        old ? old->bindings : NULL,
        &u->scope_arena
    );
    hashmap_put(sc->bindings, name, st);
    next->active_scope = sc;
    /* Emit STEP_DECLARE */
    Step *s = arena_alloc(&u->step_arena, sizeof(Step));
    if (!s) {
        return NULL;
    }
    s->kind   = STEP_DECLARE;
    s->origin = origin;
    s->info   = st->id;
    next->step = s;
    /* Link timeline */
    next->prev = prev;
    prev->next = next;
    u->current = next;
    u->tail = next;
    u->current_time = next->time;
    return next;
}
/*
 * Use (read) a variable by name.
 *
 * This resolves the variable in the current scope chain,
 * clones the current World, and links history.
 */
World *universe_use_variable(
    Universe *u,
    const char *name,
    void *origin
)
{
    if (!u || !u->current || !name) {
        return NULL;
    }
    World *prev = u->current;
    /* Resolve name in current scope chain */
    Scope *sc = prev->active_scope;
    Storage *st = NULL;
    while (sc) {
        if (sc->bindings) {
            st = hashmap_get(sc->bindings, name);
            if (st) {
                break;
            }
        }
        sc = sc->parent;
    }
    /* Clone world regardless — we record the attempt */
    World *next = world_clone(u, prev);
    if (!next) {
        return NULL;
    }
    next->time = prev->time + 1;
    /* Emit STEP_USE */
    Step *s = arena_alloc(&u->step_arena, sizeof(Step));
    if (!s) {
        return NULL;
    }
    s->kind   = STEP_USE;
    s->origin = origin;
    if (st) {
        /* Valid use */
        s->info = st->id;
    } else {
        /* Unresolved use — semantic error */
        s->info = UINT64_MAX;
    }
    next->step = s;
    /* Link timeline */
    next->prev = prev;
    prev->next = next;
    u->current = next;
    u->tail = next;
    u->current_time = next->time;
    return next;
}
//@source src/executor/world.c
#include <stdlib.h>
#include "executor/world.h"
#include "executor/universe.h"
#include "executor/step.h"
/*
 * Create the initial World.
 *
 * This represents time = 0.
 * No scopes, no stack, no memory.
 */
World *world_create_initial(Universe *u)
{
    if (!u) {
        return NULL;
    }
    World *w = arena_alloc(&u->world_arena, sizeof(World));
    if (!w) {
        return NULL;
    }
    w->time = 0;
    w->active_scope = NULL;
    w->call_stack   = NULL;
    w->memory       = NULL;
    w->step         = NULL;
    w->prev = NULL;
    w->next = NULL;
    return w;
}
/*
 * Clone a World.
 *
 * This performs a shallow copy of semantic state.
 * Ownership remains with the Universe.
 *
 * Time, links, and causality are adjusted by the caller.
 */
World *world_clone(Universe *u, const World *src)
{
    if (!u || !src) {
        return NULL;
    }
    World *w = arena_alloc(&u->world_arena, sizeof(World));
    if (!w) {
        return NULL;
    }
    w->time         = src->time;
    w->active_scope = src->active_scope;
    w->call_stack   = src->call_stack;
    w->memory       = src->memory;
    w->step         = src->step;
    w->prev = NULL;
    w->next = NULL;
    return w;
}
//@source src/frontends/c/ast.c
#include "frontends/c/ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
ASTProgram *ast_program_new(const char *path,
                            const char *src,
                            size_t len)
{
    ASTProgram *p = calloc(1, sizeof(ASTProgram));
    if (!p) return NULL;
    p->nodes = NULL;
    p->count = 0;
    p->cap   = 0;
    p->source_path = strdup(path);
    p->source_text = strdup(src);
    p->source_len  = len;
    p->root_id = 0; /* IMPORTANT */
    return p;
}
void ast_program_free(ASTProgram *p)
{
    if (!p) return;
    /* Free block stmt arrays */
    for (size_t i = 0; i < p->count; i++) {
        ASTNode *n = &p->nodes[i];
        if (n->kind == AST_BLOCK) {
            free(n->as.block.stmt_ids);
            n->as.block.stmt_ids = NULL;
            n->as.block.stmt_count = 0;
        }
    }
    free(p->nodes);
    free(p->source_path);
    free(p->source_text);
    free(p);
}
ASTNode *ast_node_get(ASTProgram *p, uint32_t id)
{
    if (!p || id == 0) return NULL;
    size_t idx = (size_t)(id - 1);
    if (idx >= p->count) return NULL;
    return &p->nodes[idx];
}
uint32_t ast_add_node(ASTProgram *p, ASTKind kind, ASTSpan at)
{
    if (!p) return 0;
    if (p->count >= p->cap) {
        size_t ncap = (p->cap == 0) ? 8 : p->cap * 2;
        ASTNode *nn = (ASTNode *)realloc(p->nodes, ncap * sizeof(ASTNode));
        if (!nn) return 0;
        /* zero new range */
        memset(nn + p->cap, 0, (ncap - p->cap) * sizeof(ASTNode));
        p->nodes = nn;
        p->cap = ncap;
    }
    ASTNode *n = &p->nodes[p->count];
    memset(n, 0, sizeof(*n));
    n->id = (uint32_t)(p->count + 1);
    n->kind = kind;
    n->at = at;
    p->count++;
    return n->id;
}
//@source src/frontends/c/ast_dump.c
#include <stdio.h>
#include "frontends/c/ast.h"
/*
 * AST dump
 *
 * This emits the FRONTEND ARTIFACT.
 *
 * - No execution
 * - No analysis
 * - No interpretation
 *
 * This is a structural, inspectable snapshot of the AST.
 */
void ast_dump(const ASTProgram *p)
{
    printf("\n-- AST ARTIFACT --\n");
    if (!p || p->count == 0) {
        printf("node_count=0\nroot_id=0\n(empty AST)\n");
        return;
    }
    if (p->root_id == 0) {
        printf("(invalid AST: no root)\n");
        return;
    }
    printf("node_count=%zu\n", p->count);
    printf("root_id=%u\n\n", p->root_id);
    for (size_t i = 0; i < p->count; i++) {
        const ASTNode *n = &p->nodes[i];
        printf("[%u] ", n->id);
        switch (n->kind) {
        case AST_PROGRAM:
            printf("PROGRAM\n");
            break;
        case AST_FUNCTION:
            printf("FUNCTION name=%s body=%u\n",
                n->as.fn.name,
                n->as.fn.body_id);
            break;
        case AST_BLOCK:
            printf("BLOCK stmts=%zu\n",
                n->as.block.stmt_count);
            break;
        case AST_RETURN:
            printf("RETURN value=%lld\n",
                (long long)n->as.ret.value);
            break;
        default:
            printf("UNKNOWN\n");
        }
    }
}
//@source src/frontends/c/frontend.c
#include <stdio.h>
#include <stdlib.h>
#include "frontends/c/frontend.h"
#include "frontends/c/lexer.h"
#include "frontends/c/parser.h"
#include "common/file.h" 
/*
 * Parse a C source file into an ASTProgram.
 *
 * This function defines the frontend artifact boundary.
 */
ASTProgram *c_parse_file_to_ast(const char *path)
{
    char *src = NULL;
    size_t len = 0;
    if (!read_entire_file(path, &src, &len))
        return NULL;
    Lexer lx;
    lexer_init(&lx, path, src, len);
    return parse_translation_unit(&lx);
}
//@source src/frontends/c/lexer.c
#include "frontends/c/lexer.h"
#include <string.h>
#include <ctype.h>
void lexer_init(Lexer *lx, const char *path, const char *src, size_t len)
{
    lx->path = path;
    lx->src  = src;
    lx->len  = len;
    lx->pos  = 0;
}
static void skip_ws(Lexer *lx)
{
    while (lx->pos < lx->len && isspace((unsigned char)lx->src[lx->pos])) {
        lx->pos++;
    }
}
Token lexer_next(Lexer *lx)
{
    skip_ws(lx);
    if (lx->pos >= lx->len) {
        return (Token){ TOK_EOF, NULL, 0 };
    }
    const char *s = lx->src + lx->pos;
    /* Keywords */
    if (strncmp(s, "int", 3) == 0 && !isalnum(s[3])) {
        lx->pos += 3;
        return (Token){ TOK_INT, s, 3 };
    }
    if (strncmp(s, "return", 6) == 0 && !isalnum(s[6])) {
        lx->pos += 6;
        return (Token){ TOK_RETURN, s, 6 };
    }
    /* Identifier (only "main" supported) */
    if (isalpha((unsigned char)*s)) {
        size_t i = 0;
        while (isalnum((unsigned char)s[i])) i++;
        lx->pos += i;
        return (Token){ TOK_IDENT, s, i };
    }
    /* Integer literal */
    if (isdigit((unsigned char)*s)) {
        size_t i = 0;
        while (isdigit((unsigned char)s[i])) i++;
        lx->pos += i;
        return (Token){ TOK_INT_LIT, s, i };
    }
    /* Punctuation */
    lx->pos++;
    switch (*s) {
    case '(': return (Token){ TOK_LPAREN, s, 1 };
    case ')': return (Token){ TOK_RPAREN, s, 1 };
    case '{': return (Token){ TOK_LBRACE, s, 1 };
    case '}': return (Token){ TOK_RBRACE, s, 1 };
    case ';': return (Token){ TOK_SEMI,   s, 1 };
    default:  return (Token){ TOK_EOF, NULL, 0 };
    }
}
int lexer_accept(Lexer *lx, TokKind k)
{
    size_t save = lx->pos;
    Token t = lexer_next(lx);
    if (t.kind == k) return 1;
    lx->pos = save;
    return 0;
}
//@source src/frontends/c/parser.c
#include <stdlib.h>
#include <string.h>
#include "frontends/c/parser.h"
// Forward Declarations
static uint32_t parse_block(ASTProgram *p, Lexer *lx);
static uint32_t parse_statement(ASTProgram *p, Lexer *lx);
/*
 * Parse a C translation unit.
 *
 * STAGE 1.5:
 *   - minimal real parser
 *   - supports:
 *       int main() { <stmts> }
 *       int <ident> ;
 *       return <int> ;
 *
 * Produces a structural AST artifact only.
 */
ASTProgram *parse_translation_unit(Lexer *lx)
{
    ASTProgram *p = ast_program_new(lx->path, lx->src, lx->len);
    if (!p) return NULL;
    /* dummy span for now */
    ASTSpan z = { .line = 1, .col = 1 };
    /* Expect: int */
    if (!lexer_accept(lx, TOK_INT)) goto fail;
    /* Expect: main */
    Token ident = lexer_next(lx);
    if (ident.kind != TOK_IDENT) goto fail;
    /* Expect: () */
    if (!lexer_accept(lx, TOK_LPAREN)) goto fail;
    if (!lexer_accept(lx, TOK_RPAREN)) goto fail;
    /* Expect: { */
    if (!lexer_accept(lx, TOK_LBRACE)) goto fail;
    /* ---- Parse statements ---- */
    uint32_t stmts[64];
    size_t stmt_count = 0;
    for (;;) {
        if (lexer_accept(lx, TOK_RBRACE))
            break;
        uint32_t stmt = parse_statement(p, lx);
        if (stmt == 0)
            goto fail;
        stmts[stmt_count++] = stmt;
    }
    /* ---- Build structural AST ---- */
    uint32_t prog_id = ast_add_node(p, AST_PROGRAM, z);
    uint32_t fn_id   = ast_add_node(p, AST_FUNCTION, z);
    uint32_t blk_id  = ast_add_node(p, AST_BLOCK, z);
    ASTNode *fn  = ast_node_get(p, fn_id);
    ASTNode *blk = ast_node_get(p, blk_id);
    /* function name */
    {
        const char *name = "main";
        size_t len = strlen(name) + 1;
        char *owned = malloc(len);
        if (!owned) goto fail;
        memcpy(owned, name, len);
        fn->as.fn.name = owned;
    }
    fn->as.fn.body_id = blk_id;
    blk->as.block.stmt_ids = malloc(sizeof(uint32_t) * stmt_count);
    if (!blk->as.block.stmt_ids) goto fail;
    memcpy(blk->as.block.stmt_ids, stmts, sizeof(uint32_t) * stmt_count);
    blk->as.block.stmt_count = stmt_count;
    p->root_id = prog_id;
    return p;
fail:
    ast_program_free(p);
    return NULL;
}
static uint32_t parse_block(ASTProgram *p, Lexer *lx)
{
    ASTSpan z = { .line = 1, .col = 1 };
    uint32_t stmt_ids[64];
    size_t stmt_count = 0;
    for (;;) {
        /* End of block */
        if (lexer_accept(lx, TOK_RBRACE)) {
            break;
        }
        uint32_t stmt = parse_statement(p, lx);
        if (stmt == 0) {
            return 0; /* parse error */
        }
        stmt_ids[stmt_count++] = stmt;
    }
    uint32_t block_id = ast_add_node(p, AST_BLOCK, z);
    ASTNode *blk = ast_node_get(p, block_id);
    blk->as.block.stmt_ids =
        malloc(sizeof(uint32_t) * stmt_count);
    if (!blk->as.block.stmt_ids)
        return 0;
    memcpy(
        blk->as.block.stmt_ids,
        stmt_ids,
        sizeof(uint32_t) * stmt_count
    );
    blk->as.block.stmt_count = stmt_count;
    return block_id;
}
static uint32_t parse_statement(ASTProgram *p, Lexer *lx)
{
    ASTSpan z = { .line = 1, .col = 1 };
    /* Nested block */
    if (lexer_accept(lx, TOK_LBRACE)) {
        return parse_block(p, lx);
    }
    /* int <ident> ; */
    if (lexer_accept(lx, TOK_INT)) {
        Token id = lexer_next(lx);
        if (id.kind != TOK_IDENT)
            return 0;
        if (!lexer_accept(lx, TOK_SEMI))
            return 0;
        uint32_t node_id = ast_add_node(p, AST_VAR_DECL, z);
        ASTNode *vd = ast_node_get(p, node_id);
        vd->as.vdecl.name = strndup(id.lexeme, id.len);
        return node_id;
    }
    /* return <int> ; */
    if (lexer_accept(lx, TOK_RETURN)) {
        Token lit = lexer_next(lx);
        if (lit.kind != TOK_INT_LIT)
            return 0;
        if (!lexer_accept(lx, TOK_SEMI))
            return 0;
        uint32_t node_id = ast_add_node(p, AST_RETURN, z);
        ASTNode *r = ast_node_get(p, node_id);
        r->as.ret.value = 0;
        return node_id;
    }
    /* <ident> ; → variable use */
    {
        size_t save = lx->pos;
        Token id = lexer_next(lx);
        if (id.kind == TOK_IDENT && lexer_accept(lx, TOK_SEMI)) {
            uint32_t node_id = ast_add_node(p, AST_VAR_USE, z);
            ASTNode *vu = ast_node_get(p, node_id);
            vu->as.vuse.name = strndup(id.lexeme, id.len);
            return node_id;
        }
        lx->pos = save;
    }
    return 0;
}
//@source src/main.c
#include <stdio.h>
#include <string.h>
#include "executor/universe.h"
#include "executor/world.h"
#include "executor/step.h"
#include "executor/scope.h"
#include "analyzer/trace.h"
#include "analyzer/use.h"
#include "analyzer/shadow.h"
#include "frontends/c/ast.h"
#include "frontends/c/frontend.h"
#include "analyzer/lifetime.h"
#include "executor/executor.h"
#include "analyzer/diagnostic.h"
/*
 * Liminal CLI entry point
 *
 * This file is intentionally thin.
 * It exists only to orchestrate the high-level pipeline:
 *
 *   source -> AST -> Worlds -> Analysis -> Artifacts
 *
 * No semantic logic belongs here.
 * No execution details belong here.
 * No analysis logic belongs here.
 *
 * This file should remain boring forever.
 */
static void print_usage(const char *prog)
{
    printf("Usage: %s <command> [args]\n", prog);
    printf("\n");
    printf("Commands:\n");
    printf("  run <file>        Execute a program into a world delta set\n");
    printf("  analyze <path>    Analyze world delta sets\n");
    printf("\n");
}
/*
 * Command: run
 *
 * Demonstration scaffold:
 *   - parse AST
 *   - build execution artifact
 *   - dump artifacts
 *   - run selected analysis passes
 */
static int cmd_run(const char *path)
{
    if (!path) {
        fprintf(stderr, "error: no input file\n");
        return 1;
    }
    /* ---- FRONTEND ---- */
    ASTProgram *ast = c_parse_file_to_ast(path);
    if (!ast) {
        fprintf(stderr, "failed to parse AST\n");
        return 1;
    }
    ast_dump(ast);
    /* ---- EXECUTOR ---- */
    Universe *u = executor_build(ast);
    if (!u) {
        fprintf(stderr, "failed to build execution artifact\n");
        ast_program_free(ast);
        return 1;
    }
    executor_dump(u);
    /* ---- ANALYSIS (Step 3.6) ---- */
    DiagnosticArtifact a = analyze_diagnostics(u->head);
    diagnostic_dump(&a);
    /* ---- CLEANUP ---- */
    ast_program_free(ast);
    return 0;
}
/*
 * Command: analyze
 *
 * Placeholder for future offline analysis pipeline.
 */
static int cmd_analyze(const char *path)
{
    (void)path;
    printf("analyze: coming soon\n");
    return 0;
}
int main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage(argv[0]);
        printf("\ncoming soon\n");
        return 0;
    }
    if (strcmp(argv[1], "run") == 0) {
        if (argc < 3) {
            printf("error: missing input file\n");
            return 1;
        }
        return cmd_run(argv[2]);
    }
    if (strcmp(argv[1], "analyze") == 0) {
        if (argc < 3) {
            printf("error: missing artifact path\n");
            return 1;
        }
        return cmd_analyze(argv[2]);
    }
    printf("unknown command: %s\n", argv[1]);
    print_usage(argv[0]);
    printf("\ncoming soon\n");
    return 1;
}
