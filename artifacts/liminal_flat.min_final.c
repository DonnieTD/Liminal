/* LIMINAL_FLAT_MIN 20260101T125910Z */
/* -------- HEADERS -------- */
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
//@header src/common/fs.h
#ifndef LIMINAL_FS_H
#define LIMINAL_FS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
bool fs_mkdir_if_missing(const char *path);
bool fs_write_file(const char *path, const char *data, size_t len);
FILE *fs_open_file(const char *path);
#endif
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
 * Enumerates semantic events in execution time.
 *
 * A Step answers:
 *   "What happened at this moment?"
 *
 * It does NOT encode:
 *   - control flow
 *   - data flow
 *   - ownership
 *
 * Those are derived later.
 */
typedef enum StepKind {
    STEP_UNKNOWN = 0,
    /* Program structure */
    STEP_ENTER_PROGRAM,
    STEP_EXIT_PROGRAM,
    STEP_ENTER_FUNCTION,
    STEP_EXIT_FUNCTION,
    /* Control flow */
    STEP_CALL,
    STEP_RETURN,
    /* Scopes */
    STEP_ENTER_SCOPE,
    STEP_EXIT_SCOPE,
    /* Variables */
    STEP_DECLARE,
    STEP_USE,
    STEP_ASSIGN,
    /* Memory (future) */
    STEP_LOAD,
    STEP_STORE,
    /* Catch-all */
    STEP_OTHER
} StepKind;
/*
 * Step
 *
 * A Step is a single semantic cause marker.
 *
 * Invariants:
 *  - Immutable once attached to a World
 *  - Owned by the Universe
 *  - Does NOT own memory
 *
 * Interpretation rules:
 *  - `origin` is opaque (usually ASTNode*)
 *  - `info` meaning depends on `kind`
 *
 * info semantics by kind:
 *  - STEP_ENTER_SCOPE / EXIT_SCOPE → scope_id
 *  - STEP_DECLARE / STEP_USE       → storage_id (or UINT64_MAX)
 *  - otherwise                     → unused (0)
 */
typedef struct Step {
    StepKind kind;
    void    *origin;
    uint64_t info;
} Step;
/*
 * Canonical stringification.
 *
 * Executor-owned.
 * Must stay stable for tooling.
 */
const char *step_kind_name(StepKind kind);
/*
 * Compatibility helper
 */
static inline const char *step_kind_str(StepKind kind)
{
    return step_kind_name(kind);
}
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
//@header src/analyzer/artifact_emit.h
#ifndef LIMINAL_ARTIFACT_EMIT_H
#define LIMINAL_ARTIFACT_EMIT_H
#include "analyzer/diagnostic.h"
struct World;
typedef struct {
    const char   *root;
    const char   *run_id;
    const char   *input_path;
    unsigned long started_at;
    const struct World *world_head;
} ArtifactContext;
void artifact_emit_all(
    const ArtifactContext *ctx,
    const DiagnosticArtifact *diagnostics
);
#endif
//@header src/analyzer/constraint.h
#pragma once
#include <stdint.h>
#include <stddef.h>
/* Stage 5.1 forward declaration */
struct SourceAnchor;
/*
 * ConstraintKind
 *
 * Enumerates semantic invariants derived from execution.
 *
 * Constraints are NOT diagnostics.
 * They express facts about semantic reality.
 */
typedef enum ConstraintKind {
    CONSTRAINT_USE_REQUIRES_DECLARATION,
    CONSTRAINT_REDECLARATION,
    CONSTRAINT_SHADOWING
} ConstraintKind;
/*
 * Constraint
 *
 * A single semantic invariant statement.
 */
typedef struct Constraint {
    ConstraintKind kind;
    uint64_t time;
    uint64_t scope_id;
    uint64_t storage_id;
    struct SourceAnchor *anchor;  /* may be NULL */
} Constraint;
/*
 * ConstraintArtifact
 *
 * Output of the constraint engine.
 */
typedef struct ConstraintArtifact {
    Constraint *items;
    size_t count;
} ConstraintArtifact;
//@header src/analyzer/constraint_declaration.h
//@header src/analyzer/constraint_declaration.h
#ifndef LIMINAL_CONSTRAINT_DECLARATION_H
#define LIMINAL_CONSTRAINT_DECLARATION_H
#include "analyzer/constraint.h"
struct World;
/*
 * Declaration-related constraint extraction.
 *
 * Emits:
 *   - CONSTRAINT_REDECLARATION
 *   - CONSTRAINT_SHADOWING
 */
ConstraintArtifact analyze_declaration_constraints(struct World *head);
#endif /* LIMINAL_CONSTRAINT_DECLARATION_H */
//@header src/analyzer/constraint_diagnostic.h
#pragma once
#include <stddef.h>
#include "analyzer/constraint.h"
#include "analyzer/diagnostic.h"
/*
 * Project constraints into diagnostics.
 *
 * Returns number of diagnostics written.
 */
size_t constraint_to_diagnostic(
    const ConstraintArtifact *constraints,
    Diagnostic *out,
    size_t cap
);
//@header src/analyzer/constraint_engine.h
#ifndef LIMINAL_CONSTRAINT_ENGINE_H
#define LIMINAL_CONSTRAINT_ENGINE_H
#include "analyzer/constraint.h"
#include "executor/world.h"
/*
 * Constraint engine entry point.
 *
 * Consumes a World timeline and produces semantic constraints.
 */
ConstraintArtifact analyze_constraints(struct World *head);
#endif /* LIMINAL_CONSTRAINT_ENGINE_H */
//@header src/analyzer/constraint_variable.h
#ifndef LIMINAL_CONSTRAINT_VARIABLE_H
#define LIMINAL_CONSTRAINT_VARIABLE_H
#include "analyzer/constraint.h"
#include "executor/world.h"
/*
 * Variable-related constraint extraction.
 *
 * Emits:
 *   - CONSTRAINT_REDECLARATION
 */
ConstraintArtifact analyze_variable_constraints(struct World *head);
#endif
//@header src/analyzer/diagnostic.h
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "analyzer/source_anchor.h"
#include "analyzer/diagnostic_id.h"
struct ASTNode;
typedef enum DiagnosticKind {
    DIAG_REDECLARATION = 0,
    DIAG_SHADOWING,
    DIAG_USE_BEFORE_DECLARE,
    DIAG_USE_AFTER_SCOPE_EXIT,
    /* Sentinel */
    DIAG_KIND_MAX
} DiagnosticKind;
typedef struct Diagnostic {
    DiagnosticId id;
    DiagnosticKind kind;
    uint64_t time;
    uint64_t scope_id;
    uint64_t prev_scope;
    struct SourceAnchor *anchor;
} Diagnostic;
typedef struct DiagnosticArtifact {
    Diagnostic *items;
    size_t count;
} DiagnosticArtifact;
struct World;
DiagnosticArtifact analyze_diagnostics(struct World *head);
void diagnostic_dump(const DiagnosticArtifact *a);
const char *diagnostic_kind_name(DiagnosticKind k);
//@header src/analyzer/diagnostic_id.h
#ifndef LIMINAL_DIAGNOSTIC_ID_H
#define LIMINAL_DIAGNOSTIC_ID_H
#include <stdint.h>
#include "analyzer/constraint.h"
/*
 * DiagnosticId
 *
 * Stable semantic identity for a diagnostic.
 */
typedef struct DiagnosticId {
    uint64_t value;
} DiagnosticId;
/*
 * Derive a stable diagnostic identity from a constraint.
 */
DiagnosticId diagnostic_id_from_constraint(const Constraint *c);
/*
 * Render a diagnostic id (human-readable, stable).
 *
 * NOTE:
 *  - Formatting is centralized here
 *  - Consumers must not interpret DiagnosticId internals
 */
void diagnostic_id_render(DiagnosticId id);
#endif /* LIMINAL_DIAGNOSTIC_ID_H */
//@header src/analyzer/diagnostic_project.h
#ifndef LIMINAL_DIAGNOSTIC_PROJECT_H
#define LIMINAL_DIAGNOSTIC_PROJECT_H
#include "analyzer/diagnostic.h"
#include <stdio.h>
void diagnostic_project_ndjson(
    const DiagnosticArtifact *a,
    FILE *out
);
#endif
//@header src/analyzer/diagnostic_serialize.h
#ifndef LIMINAL_DIAGNOSTIC_SERIALIZE_H
#define LIMINAL_DIAGNOSTIC_SERIALIZE_H
#include <stdio.h>
#include "analyzer/diagnostic.h"
/*
 * Serialize diagnostics to NDJSON.
 */
void diagnostic_serialize_ndjson(
    FILE *out,
    const DiagnosticArtifact *a
);
/*
 * Deserialize a single diagnostic from an NDJSON stream.
 *
 * Returns:
 *   1 on success
 *   0 on EOF or parse failure
 *
 * Does NOT allocate.
 */
int diagnostic_deserialize_line(
    FILE *in,
    Diagnostic *out
);
#endif /* LIMINAL_DIAGNOSTIC_SERIALIZE_H */
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
//@header src/analyzer/source_anchor.h
/* LIMINAL_FLAT_MIN 20251231T175500Z */
//@header src/analyzer/source_anchor.h
#ifndef LIMINAL_SOURCE_ANCHOR_H
#define LIMINAL_SOURCE_ANCHOR_H
#include <stdint.h>
/*
 * SourceAnchor
 *
 * Stable reference into frontend artifacts.
 *
 * This is a POINTER ONLY.
 * It does not own memory.
 */
typedef struct SourceAnchor {
    const char *file;    /* ASTProgram->source_path (optional for now) */
    uint32_t    node_id; /* ASTNode->id */
    uint32_t    line;
    uint32_t    col;
} SourceAnchor;
#endif /* LIMINAL_SOURCE_ANCHOR_H */
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
//@header src/consumers/cause_key.h
#ifndef LIMINAL_CAUSE_KEY_H
#define LIMINAL_CAUSE_KEY_H
#include <stdint.h>
#include "executor/step.h"
typedef struct CauseKey {
    StepKind step;
    uint64_t ast_id;
    uint64_t scope_id;
} CauseKey;
/* MUST be exported — used by convergence_map */
int cause_key_equal(const CauseKey *a, const CauseKey *b);
#endif /* LIMINAL_CAUSE_KEY_H */
//@header src/consumers/cause_key_extract.h
#ifndef LIMINAL_CAUSE_KEY_EXTRACT_H
#define LIMINAL_CAUSE_KEY_EXTRACT_H
#include "consumers/root_chain.h"
#include "consumers/cause_key.h"
/*
 * Extract a stable semantic cause key from a root chain.
 *
 * Returns:
 *   0 on success
 *  -1 if no valid cause key can be derived
 *
 * PURE:
 *  - no allocation
 *  - no mutation
 */
int extract_cause_key(
    const RootChain *chain,
    CauseKey *out
);
#endif /* LIMINAL_CAUSE_KEY_EXTRACT_H */
//@header src/consumers/convergence_build.h
#ifndef LIMINAL_CONVERGENCE_BUILD_H
#define LIMINAL_CONVERGENCE_BUILD_H
#include "analyzer/diagnostic.h"
#include "consumers/root_chain.h"
#include "consumers/convergence_map.h"
/*
 * Build convergence groups from diagnostics + root chains.
 *
 * PURE:
 *  - no allocation outside ConvergenceMap
 *  - deterministic
 */
int build_convergence_map(
    const DiagnosticArtifact *diags,
    const RootChain *chains,
    ConvergenceMap *out
);
#endif /* LIMINAL_CONVERGENCE_BUILD_H */
//@header src/consumers/convergence_map.h
#ifndef LIMINAL_CONVERGENCE_MAP_H
#define LIMINAL_CONVERGENCE_MAP_H
#include <stddef.h>
#include "consumers/cause_key.h"
#include "analyzer/diagnostic.h"
/*
 * A convergence entry groups diagnostics that share
 * an identical semantic cause key.
 */
typedef struct ConvergenceEntry {
    CauseKey key;
    const Diagnostic **diagnostics;
    size_t count;
    size_t capacity;
} ConvergenceEntry;
/*
 * A convergence map groups all convergence entries
 * discovered in a run.
 */
typedef struct ConvergenceMap {
    ConvergenceEntry *entries;
    size_t count;
    size_t capacity;
} ConvergenceMap;
void convergence_map_add(
    ConvergenceMap *map,
    const CauseKey *key,
    const Diagnostic *diag
);
#endif /* LIMINAL_CONVERGENCE_MAP_H */
//@header src/consumers/convergence_render.h
#ifndef LIMINAL_CONVERGENCE_RENDER_H
#define LIMINAL_CONVERGENCE_RENDER_H
#include "consumers/convergence_map.h"
/*
 * Render cross-diagnostic convergence to stdout.
 */
void render_convergence(const ConvergenceMap *map);
#endif /* LIMINAL_CONVERGENCE_RENDER_H */
//@header src/consumers/diagnostic_anchor.h
#ifndef LIMINAL_DIAGNOSTIC_ANCHOR_H
#define LIMINAL_DIAGNOSTIC_ANCHOR_H
#include "analyzer/diagnostic.h"
#include "consumers/timeline_event.h"
typedef struct DiagnosticAnchor {
    DiagnosticId id;
    uint64_t diagnostic_time;
    TimelineEvent cause;
} DiagnosticAnchor;
#endif
//@header src/consumers/diagnostic_cause.h
#ifndef LIMINAL_DIAGNOSTIC_CAUSE_H
#define LIMINAL_DIAGNOSTIC_CAUSE_H
#include "consumers/root_cause.h"
typedef struct DiagnosticCause {
    uint64_t diagnostic_id;
    RootCause cause;
} DiagnosticCause;
#endif
//@header src/consumers/diagnostic_diff.h
#pragma once
#include "analyzer/diagnostic.h"
typedef enum {
    DIFF_ADDED,
    DIFF_REMOVED,
    DIFF_UNCHANGED
} DiagnosticDiffKind;
typedef struct {
    DiagnosticDiffKind kind;
    DiagnosticId id;
} DiagnosticDiff;
size_t diagnostic_diff(
    const DiagnosticArtifact *old_run,
    const DiagnosticArtifact *new_run,
    DiagnosticDiff *out,
    size_t cap
);
//@header src/consumers/diagnostic_render.h
#pragma once
#include <stdio.h>
struct DiagnosticArtifact;
void diagnostic_render_terminal(
    const struct DiagnosticArtifact *a,
    FILE *out
);
//@header src/consumers/diagnostic_stats.h
#ifndef LIMINAL_DIAGNOSTIC_STATS_H
#define LIMINAL_DIAGNOSTIC_STATS_H
#include <stdio.h>   /* FILE */
#include <stddef.h>  /* size_t */
#include "analyzer/diagnostic.h"
typedef struct {
    size_t total;
    size_t by_kind[DIAG_KIND_MAX];
} DiagnosticStats;
/*
 * Compute statistics from diagnostics.
 * Does NOT allocate.
 */
void diagnostic_stats_compute(
    const DiagnosticArtifact *a,
    DiagnosticStats *out
);
/*
 * Emit statistics to a stream.
 * Does NOT allocate.
 */
void diagnostic_stats_emit(
    const DiagnosticStats *stats,
    FILE *out
);
#endif /* LIMINAL_DIAGNOSTIC_STATS_H */
//@header src/consumers/diagnostic_timeline.h
#pragma once
#include <stdio.h>
void diagnostic_timeline_render(FILE *in, FILE *out);
//@header src/consumers/diagnostic_validate.h
#pragma once
#include "analyzer/diagnostic.h"
typedef enum {
    VALIDATION_DUPLICATE_ID,
    VALIDATION_NON_MONOTONIC_TIME
} ValidationIssueKind;
typedef struct {
    ValidationIssueKind kind;
    DiagnosticId id;
} ValidationIssue;
size_t validate_diagnostics(
    const DiagnosticArtifact *a,
    ValidationIssue *out,
    size_t cap
);
//@header src/consumers/fix_surface.h
#pragma once
#include <stddef.h>
#include "consumers/cause_key.h"
typedef struct {
    CauseKey *causes;
    size_t count;
    size_t capacity;
} FixSurface;
//@header src/consumers/fix_surface_build.h
#ifndef LIMINAL_FIX_SURFACE_BUILD_H
#define LIMINAL_FIX_SURFACE_BUILD_H
#include "consumers/convergence_map.h"
#include "consumers/fix_surface.h"
/*
 * Extract minimal fix surface from convergence map.
 *
 * PURE:
 *  - deterministic
 *  - no mutation
 */
FixSurface build_fix_surface(const ConvergenceMap *map);
#endif /* LIMINAL_FIX_SURFACE_BUILD_H */
//@header src/consumers/fix_surface_render.h
#ifndef LIMINAL_FIX_SURFACE_RENDER_H
#define LIMINAL_FIX_SURFACE_RENDER_H
#include "consumers/fix_surface.h"
/*
 * Render fix surface to stdout.
 */
void render_fix_surface(const FixSurface *fs);
#endif /* LIMINAL_FIX_SURFACE_RENDER_H */
//@header src/consumers/root_cause.h
#ifndef LIMINAL_ROOT_CAUSE_H
#define LIMINAL_ROOT_CAUSE_H
#include <stdint.h>
typedef enum RootCauseKind {
    ROOT_CAUSE_DECLARATION,
    ROOT_CAUSE_USE,
    ROOT_CAUSE_SCOPE_ENTRY,
    ROOT_CAUSE_SCOPE_EXIT,
    ROOT_CAUSE_PREVIOUS_DIAGNOSTIC,
    ROOT_CAUSE_UNKNOWN
} RootCauseKind;
typedef struct RootCause {
    RootCauseKind kind;
    uint64_t time;
    uint64_t ast_id;
    uint64_t scope_id;
} RootCause;
#endif
//@header src/consumers/root_cause_extract.h
#ifndef LIMINAL_ROOT_CAUSE_EXTRACT_H
#define LIMINAL_ROOT_CAUSE_EXTRACT_H
#include "executor/world.h"
#include "analyzer/diagnostic.h"
#include "consumers/root_cause.h"
RootCause root_cause_extract(
    const struct World *head,
    const struct Diagnostic *d
);
#endif
//@header src/consumers/root_chain.h
#ifndef LIMINAL_ROOT_CHAIN_H
#define LIMINAL_ROOT_CHAIN_H
#include <stddef.h>
#include <stdint.h>
#include "common/arena.h"              // ✅ REQUIRED
#include "executor/step.h"
#include "executor/world.h"
#include "analyzer/diagnostic.h"
#include "analyzer/diagnostic_id.h"
/*
 * RootRole
 *
 * Structural role a step plays in a diagnostic chain.
 */
typedef enum RootRole {
    ROOT_ROLE_CAUSE,
    ROOT_ROLE_AMPLIFIER,
    ROOT_ROLE_WITNESS,
    ROOT_ROLE_SUPPRESSOR
} RootRole;
typedef struct RootChainNode {
    uint64_t time;
    StepKind step;
    uint64_t ast_id;
    uint64_t scope_id;
    RootRole role;
} RootChainNode;
typedef struct RootChain {
    DiagnosticId diagnostic_id;
    RootChainNode *nodes;
    size_t count;
} RootChain;
/*
 * Build a root-cause chain by walking the World timeline backwards.
 *
 * PURE:
 *  - no mutation
 *  - arena-only allocation
 */
RootChain build_root_chain(
    Arena *arena,
    const World *head,
    const Diagnostic *diag
);
/* Render only */
void render_root_chain(const RootChain *chain);
#endif /* LIMINAL_ROOT_CHAIN_H */
//@header src/consumers/root_chain_role.h
#ifndef LIMINAL_ROOT_CHAIN_ROLE_H
#define LIMINAL_ROOT_CHAIN_ROLE_H
#include "consumers/root_chain.h"
#include "analyzer/diagnostic.h"
/*
 * Assign semantic roles to nodes in a root-cause chain.
 *
 * nodes[0] is the closest causal event to the diagnostic.
 */
void assign_root_chain_roles(
    RootChain *chain,
    DiagnosticKind kind
);
#endif /* LIMINAL_ROOT_CHAIN_ROLE_H */
//@header src/consumers/run_artifact.h
#ifndef LIMINAL_RUN_ARTIFACT_H
#define LIMINAL_RUN_ARTIFACT_H
#include <stddef.h>
#include "analyzer/diagnostic.h"
/*
 * Loaded run snapshot.
 *
 * Immutable after load.
 */
typedef struct RunArtifact {
    char *run_id;
    char *input_path;
    unsigned long started_at;
    DiagnosticArtifact diagnostics;
    /* Timeline (opaque for now) */
    struct {
        unsigned long time;
        int step;
        unsigned int ast;
    } *timeline;
    size_t timeline_count;
} RunArtifact;
#endif /* LIMINAL_RUN_ARTIFACT_H */
//@header src/consumers/run_contract.h
#ifndef LIMINAL_RUN_CONTRACT_H
#define LIMINAL_RUN_CONTRACT_H
/*
 * Artifact presence contract.
 *
 * Required:
 *   - meta.json
 *   - diagnostics.ndjson
 *
 * Optional:
 *   - timeline.ndjson
 */
typedef struct RunContract {
    int require_meta;
    int require_diagnostics;
    int allow_missing_timeline;
} RunContract;
/* Canonical Stage 7.1 contract */
static const RunContract LIMINAL_RUN_CONTRACT = {
    .require_meta        = 1,
    .require_diagnostics = 1,
    .allow_missing_timeline = 1
};
#endif /* LIMINAL_RUN_CONTRACT_H */
//@header src/consumers/run_descriptor.h
#ifndef LIMINAL_RUN_DESCRIPTOR_H
#define LIMINAL_RUN_DESCRIPTOR_H
/*
 * RunDescriptor
 *
 * Pure structural identity for a run directory.
 * No IO, no validation, no semantics.
 */
typedef struct RunDescriptor {
    const char *root_dir;
    const char *run_id;
    const char *meta_path;
    const char *diagnostics_path;
    const char *timeline_path;
} RunDescriptor;
#endif /* LIMINAL_RUN_DESCRIPTOR_H */
//@header src/consumers/scope_alignment.h
#ifndef LIMINAL_SCOPE_ALIGNMENT_H
#define LIMINAL_SCOPE_ALIGNMENT_H
#include <stdint.h>
typedef enum ScopeChangeKind {
    SCOPE_UNCHANGED,
    SCOPE_ADDED,
    SCOPE_REMOVED,
    SCOPE_SPLIT,
    SCOPE_MERGED,
    SCOPE_MOVED
} ScopeChangeKind;
typedef struct ScopeAlignment {
    uint64_t old_sig;
    uint64_t new_sig;
    ScopeChangeKind kind;
} ScopeAlignment;
#endif
//@header src/consumers/scope_graph.h
#ifndef LIMINAL_SCOPE_GRAPH_H
#define LIMINAL_SCOPE_GRAPH_H
#include <stdint.h>
#include <stddef.h>
typedef struct ScopeNode {
    uint64_t scope_id;
    uint64_t parent_id;     /* 0 = root */
    uint64_t enter_time;
    uint64_t exit_time;     /* UINT64_MAX if open */
} ScopeNode;
typedef struct ScopeGraph {
    ScopeNode *nodes;
    size_t count;
} ScopeGraph;
#endif /* LIMINAL_SCOPE_GRAPH_H */
//@header src/consumers/scope_graph_extract.h
#ifndef LIMINAL_SCOPE_GRAPH_EXTRACT_H
#define LIMINAL_SCOPE_GRAPH_EXTRACT_H
#include "executor/world.h"
#include "consumers/scope_graph.h"
ScopeGraph scope_graph_extract(
    const struct World *head
);
#endif
//@header src/consumers/scope_signature.h
#ifndef LIMINAL_SCOPE_SIGNATURE_H
#define LIMINAL_SCOPE_SIGNATURE_H
#include <stdint.h>
#include "consumers/scope_graph.h"
uint64_t scope_signature(const ScopeNode *n);
#endif
//@header src/consumers/semantic_cause_diff.h
#ifndef LIMINAL_SEMANTIC_CAUSE_DIFF_H
#define LIMINAL_SEMANTIC_CAUSE_DIFF_H
#include "consumers/semantic_diff.h"
#include "consumers/diagnostic_anchor.h"
typedef enum CauseChangeKind {
    CAUSE_NONE,
    CAUSE_NEW_STEP,
    CAUSE_STEP_MOVED,
    CAUSE_STEP_REMOVED
} CauseChangeKind;
typedef struct SemanticCauseDiff {
    SemanticDiff base;
    CauseChangeKind cause;
    uint32_t old_step;
    uint32_t new_step;
} SemanticCauseDiff;
#endif
//@header src/consumers/semantic_diff.h
#ifndef LIMINAL_SEMANTIC_DIFF_H
#define LIMINAL_SEMANTIC_DIFF_H
#include <stddef.h>
#include "analyzer/diagnostic.h"
typedef enum SemanticDiffKind {
    SEMDIFF_ADDED,
    SEMDIFF_REMOVED,
    SEMDIFF_UNCHANGED,
    SEMDIFF_MOVED   /* same id, different time */
} SemanticDiffKind;
typedef struct SemanticDiff {
    SemanticDiffKind kind;
    DiagnosticId id;
    uint64_t old_time;
    uint64_t new_time;
} SemanticDiff;
/*
 * Compute semantic diff between two runs.
 *
 * No allocation.
 * Deterministic.
 * Stable ordering.
 */
size_t semantic_diff(
    const DiagnosticArtifact *old_run,
    const DiagnosticArtifact *new_run,
    SemanticDiff *out,
    size_t cap
);
#endif /* LIMINAL_SEMANTIC_DIFF_H */
//@header src/consumers/timeline_diff.h
#ifndef LIMINAL_TIMELINE_DIFF_H
#define LIMINAL_TIMELINE_DIFF_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
/*
 * TimelineDiffKind
 *
 * Describes how a timeline step changed between two runs.
 */
typedef enum {
    TIMELINE_DIFF_UNCHANGED,
    TIMELINE_DIFF_ADDED,
    TIMELINE_DIFF_REMOVED,
    TIMELINE_DIFF_CHANGED
} TimelineDiffKind;
/*
 * TimelineStepView
 *
 * Minimal, normalized view of a timeline step.
 * Mirrors timeline.ndjson fields exactly.
 */
typedef struct {
    uint64_t time;
    uint32_t step_kind;
    uint32_t ast_id;
} TimelineStepView;
/*
 * TimelineDiff
 *
 * Represents a single divergence in semantic time.
 *
 * Invariants:
 *  - Deterministic
 *  - No allocation
 *  - One entry per divergent time index
 */
typedef struct {
    TimelineDiffKind kind;
    uint64_t time;
    /* Present for ADDED / REMOVED / CHANGED */
    TimelineStepView before;
    TimelineStepView after;
} TimelineDiff;
/*
 * timeline_diff
 *
 * Compare two normalized timeline streams.
 *
 * Rules:
 *  - Comparison is by `time` index
 *  - No sorting
 *  - No allocation
 *  - Deterministic output
 *
 * Returns:
 *  - Number of diff entries written to `out`
 */
size_t timeline_diff(
    const TimelineStepView *old_steps,
    size_t old_count,
    const TimelineStepView *new_steps,
    size_t new_count,
    TimelineDiff *out,
    size_t cap
);
/*
 * timeline_diff_first_line
 *
 * Temporary / bootstrap timeline diff.
 *
 * Returns:
 *  - (size_t)-1 if timelines are identical
 *  - first differing line index otherwise
 *
 * NOTE:
 *  - Deterministic
 *  - No allocation
 *  - Used to prove Stage 7 exit criteria
 */
size_t timeline_diff_first_line(
    FILE *a,
    FILE *b
);
#endif /* LIMINAL_TIMELINE_DIFF_H */
//@header src/consumers/timeline_emit.h
#ifndef LIMINAL_TIMELINE_EMIT_H
#define LIMINAL_TIMELINE_EMIT_H
#include <stdio.h>
struct World;
/* Human / tool readable timeline */
void emit_timeline(
    const struct World *head,
    FILE *out
);
/* NDJSON artifact timeline */
void timeline_emit_ndjson(
    const struct World *head,
    FILE *out
);
#endif /* LIMINAL_TIMELINE_EMIT_H */
//@header src/consumers/timeline_event.h
#ifndef LIMINAL_TIMELINE_EVENT_H
#define LIMINAL_TIMELINE_EVENT_H
#include <stdint.h>
typedef struct TimelineEvent {
    uint64_t time;
    uint32_t step_kind;
    uint32_t ast_id;
} TimelineEvent;
#endif /* LIMINAL_TIMELINE_EVENT_H */
//@header src/consumers/timeline_extract.h
#ifndef LIMINAL_TIMELINE_EXTRACT_H
#define LIMINAL_TIMELINE_EXTRACT_H
#include <stddef.h>
#include "executor/world.h"
#include "consumers/timeline_event.h"
size_t timeline_extract(
    const struct World *head,
    TimelineEvent *out,
    size_t cap
);
#endif
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
//@header src/commands/cmd_analyze.h
#ifndef LIMINAL_CMD_ANALYZE_H
#define LIMINAL_CMD_ANALYZE_H
int cmd_analyze(int argc, char **argv);
#endif
//@header src/commands/cmd_diff.h
#ifndef LIMINAL_CMD_DIFF_H
#define LIMINAL_CMD_DIFF_H
int cmd_diff(int argc, char **argv);
#endif /* LIMINAL_CMD_DIFF_H */
//@header src/commands/cmd_policy.h
#ifndef LIMINAL_CMD_POLICY_H
#define LIMINAL_CMD_POLICY_H
#include "policy/policy.h"
#include "analyzer/diagnostic.h"
/*
 * Print policy decision to stderr.
 * Returns non-zero on POLICY_DENY.
 */
int cmd_apply_policy(
    const Policy *policy,
    const DiagnosticArtifact *diagnostics
);
#endif /* LIMINAL_CMD_POLICY_H */
//@header src/commands/command.h
#ifndef LIMINAL_COMMAND_H
#define LIMINAL_COMMAND_H
typedef int (*command_fn)(int argc, char **argv);
typedef struct {
    const char *name;
    int min_args;
    command_fn handler;
} CommandSpec;
/*
 * Dispatch a subcommand.
 *
 * argc/argv are expected to start at the command name.
 */
int dispatch_command(
    int argc,
    char **argv,
    const CommandSpec *commands,
    int command_count
);
#endif /* LIMINAL_COMMAND_H */
//@header src/policy/default_policy.h
#ifndef LIMINAL_DEFAULT_POLICY_H
#define LIMINAL_DEFAULT_POLICY_H
#include "policy/policy.h"
/*
 * Default Liminal policy.
 *
 * Philosophy:
 *  - Structural violations deny
 *  - Shadowing warns
 *  - Redeclaration denies
 *  - Use-before-declare denies
 */
extern const Policy LIMINAL_DEFAULT_POLICY;
#endif /* LIMINAL_DEFAULT_POLICY_H */
//@header src/policy/policy.h
#ifndef LIMINAL_POLICY_H
#define LIMINAL_POLICY_H
#include <stddef.h>
#include "analyzer/diagnostic.h"
typedef struct Policy {
    unsigned char deny_kind[DIAG_KIND_MAX];
    size_t        max_by_kind[DIAG_KIND_MAX];
    size_t        max_total;
} Policy;
typedef enum PolicyDecision {
    POLICY_ALLOW = 0,
    POLICY_WARN,
    POLICY_DENY
} PolicyDecision;
typedef struct PolicyRule {
    DiagnosticKind kind;
    size_t max_count;   /* 0 = unlimited */
    int deny;           /* boolean */
} PolicyRule;
/*
 * Apply policy to diagnostics.
 *
 * Returns:
 *   0 → allowed
 *   non-zero → policy violation
 */
int cmd_apply_policy(
    const Policy *policy,
    const DiagnosticArtifact *diagnostics
);
PolicyDecision policy_evaluate(
    const Policy *policy,
    const DiagnosticArtifact *diagnostics
);
#endif /* LIMINAL_POLICY_H */
//@header src/policy/policy_default.h
Policy policy_default(void);/* -------- SOURCES -------- */
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
//@source src/common/fs.c
#include "common/fs.h"
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
bool fs_mkdir_if_missing(const char *path)
{
    if (mkdir(path, 0755) == 0)
        return true;
    if (errno == EEXIST)
        return true;
    return false;
}
bool fs_write_file(const char *path, const char *data, size_t len)
{
    FILE *f = fopen(path, "w");
    if (!f)
        return false;
    fwrite(data, 1, len, f);
    fclose(f);
    return true;
}
FILE *fs_open_file(const char *path)
{
    return fopen(path, "w");
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
#include "executor/step.h"
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
//@source src/analyzer/artifact_emit.c
#include "analyzer/artifact_emit.h"
#include "analyzer/diagnostic_project.h"
#include "common/fs.h"
#include <stdio.h>
#include <string.h>
#include "consumers/timeline_emit.h"
#include "executor/world.h"
static void emit_meta(const ArtifactContext *ctx, const char *dir)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/meta.json", dir);
    char buf[1024];
    snprintf(
        buf, sizeof(buf),
        "{\n"
        "  \"liminal_version\": \"0.5.3\",\n"
        "  \"run_id\": \"%s\",\n"
        "  \"started_at\": %lu,\n"
        "  \"input\": \"%s\"\n"
        "}\n",
        ctx->run_id,
        ctx->started_at,
        ctx->input_path
    );
    fs_write_file(path, buf, strlen(buf));
}
static void emit_diagnostics(
    const DiagnosticArtifact *a,
    const char *dir
)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/diagnostics.ndjson", dir);
    FILE *out = fs_open_file(path);
    if (!out)
        return;
    diagnostic_project_ndjson(a, out);
    fclose(out);
}
void artifact_emit_all(
    const ArtifactContext *ctx,
    const DiagnosticArtifact *diagnostics
)
{
    char run_dir[512];
    fs_mkdir_if_missing(ctx->root);
    snprintf(run_dir, sizeof(run_dir), "%s/%s", ctx->root, ctx->run_id);
    fs_mkdir_if_missing(run_dir);
    emit_meta(ctx, run_dir);
    emit_diagnostics(diagnostics, run_dir);
    /* Timeline emission (first-class artifact) */
    {
        char path[512];
        snprintf(path, sizeof(path), "%s/timeline.ndjson", run_dir);
        FILE *out = fs_open_file(path);
        if (out) {
            timeline_emit_ndjson(ctx->world_head, out);
            fclose(out);
        }
    }
}
//@source src/analyzer/constraint_declaration.c
//@source src/analyzer/constraint_declaration.c
#include "analyzer/constraint.h"
#include "analyzer/source_anchor.h"
#include "analyzer/trace.h"
#include "executor/world.h"
#include "executor/step.h"
#include "executor/scope.h"
#include "common/hashmap.h"
#include "frontends/c/ast.h"
#include <stdlib.h>
#include <stdint.h>
static int scope_has_name(Scope *s, const char *name)
{
    return s && s->bindings && hashmap_get(s->bindings, name);
}
static SourceAnchor *anchor_from_origin(void *origin)
{
    if (!origin)
        return NULL;
    ASTNode *n = (ASTNode *)origin;
    SourceAnchor *a = calloc(1, sizeof(SourceAnchor));
    if (!a)
        return NULL;
    a->file    = NULL;          /* filled later (Stage 5.2) */
    a->node_id = n->id;
    a->line    = n->at.line;
    a->col     = n->at.col;
    return a;
}
ConstraintArtifact analyze_declaration_constraints(struct World *head)
{
    size_t cap = 64;
    Constraint *buf = calloc(cap, sizeof(Constraint));
    size_t count = 0;
    if (!buf || !head) {
        return (ConstraintArtifact){ .items = NULL, .count = 0 };
    }
    Trace t = trace_begin(head);
    while (trace_is_valid(&t)) {
        World *w = trace_current(&t);
        Step  *s = w ? w->step : NULL;
        if (!s || s->kind != STEP_DECLARE) {
            trace_next(&t);
            continue;
        }
        World *prev = w->prev;
        if (!prev || !prev->active_scope)
            goto next;
        Scope *cur = prev->active_scope;
        const char *name = NULL;
        /* Extract name from AST origin (safe for now) */
        if (s->origin) {
            ASTNode *n = (ASTNode *)s->origin;
            name = n->as.vdecl.name;
        }
        if (!name)
            goto next;
        /* 1. Redeclaration in same scope */
        if (scope_has_name(cur, name) && count < cap) {
            buf[count++] = (Constraint){
                .kind       = CONSTRAINT_REDECLARATION,
                .time       = w->time,
                .scope_id   = cur->id,
                .storage_id = s->info,
                .anchor     = anchor_from_origin(s->origin)
            };
            goto next;
        }
        /* 2. Shadowing parent scope */
        for (Scope *p = cur->parent; p; p = p->parent) {
            if (scope_has_name(p, name) && count < cap) {
                buf[count++] = (Constraint){
                    .kind       = CONSTRAINT_SHADOWING,
                    .time       = w->time,
                    .scope_id   = cur->id,
                    .storage_id = s->info,
                    .anchor     = anchor_from_origin(s->origin)
                };
                break;
            }
        }
    next:
        trace_next(&t);
    }
    return (ConstraintArtifact){
        .items = buf,
        .count = count
    };
}
//@source src/analyzer/constraint_diagnostic.c
#include "analyzer/constraint.h"
#include "analyzer/constraint_diagnostic.h"
#include "analyzer/diagnostic.h"
#include "analyzer/diagnostic_id.h"
size_t constraint_to_diagnostic(
    const ConstraintArtifact *constraints,
    Diagnostic *out,
    size_t cap
) {
    size_t count = 0;
    if (!constraints || !out || cap == 0)
        return 0;
    for (size_t i = 0; i < constraints->count && count < cap; i++) {
        const Constraint *c = &constraints->items[i];
        Diagnostic *d = &out[count];
        /* Stable identity derived from constraint */
        d->id = diagnostic_id_from_constraint(c);
        d->time      = c->time;
        d->scope_id  = c->scope_id;
        d->prev_scope = 0;
        d->anchor    = c->anchor;
        switch (c->kind) {
        case CONSTRAINT_REDECLARATION:
            d->kind = DIAG_REDECLARATION;
            d->prev_scope = c->scope_id;
            break;
        case CONSTRAINT_SHADOWING:
            d->kind = DIAG_SHADOWING;
            /* parent scope not yet surfaced */
            d->prev_scope = 0;
            break;
        case CONSTRAINT_USE_REQUIRES_DECLARATION:
            d->kind = DIAG_USE_BEFORE_DECLARE;
            break;
        default:
            /* Unknown / future constraint — ignored by design */
            continue;
        }
        count++;
    }
    return count;
}
//@source src/analyzer/constraint_engine.c
#include "analyzer/constraint_engine.h"
#include "analyzer/constraint_variable.h"
#include "analyzer/constraint_declaration.h"
#include <stdlib.h>
#include <string.h>
ConstraintArtifact analyze_constraints(struct World *head)
{
    ConstraintArtifact a = analyze_variable_constraints(head);
    ConstraintArtifact b = analyze_declaration_constraints(head);
    /* Temporary merge (Stage 4 discipline) */
    size_t total = a.count + b.count;
    Constraint *buf = calloc(total, sizeof(Constraint));
    if (!buf)
        return a;
    memcpy(buf, a.items, a.count * sizeof(Constraint));
    memcpy(buf + a.count, b.items, b.count * sizeof(Constraint));
    free(a.items);
    free(b.items);
    return (ConstraintArtifact){
        .items = buf,
        .count = total
    };
}
//@source src/analyzer/constraint_scope.c
void __constraint_scope_stub(void) {}
//@source src/analyzer/constraint_variable.c
#include "analyzer/constraint_variable.h"
#include "analyzer/trace.h"
#include "executor/world.h"
#include "executor/step.h"
#include <stdlib.h>
#include <stdint.h>
ConstraintArtifact analyze_variable_constraints(struct World *head)
{
    /* Empty artifact for degenerate cases */
    if (!head) {
        return (ConstraintArtifact){
            .items = NULL,
            .count = 0
        };
    }
    /* Fixed-cap temporary buffer (Stage 4.x discipline) */
    size_t cap = 64;
    Constraint *buf = calloc(cap, sizeof(Constraint));
    size_t count = 0;
    if (!buf) {
        return (ConstraintArtifact){
            .items = NULL,
            .count = 0
        };
    }
    Trace t = trace_begin(head);
    while (trace_is_valid(&t)) {
        World *w = trace_current(&t);
        Step  *s = w ? w->step : NULL;
        if (s && s->kind == STEP_USE) {
            /* Unresolved variable use → constraint */
            if (s->info == UINT64_MAX && count < cap) {
                buf[count++] = (Constraint){
                    .kind       = CONSTRAINT_USE_REQUIRES_DECLARATION,
                    .time       = w->time,
                    .scope_id   = 0,           /* scope not required yet */
                    .storage_id = UINT64_MAX
                };
            }
        }
        trace_next(&t);
    }
    return (ConstraintArtifact){
        .items = buf,
        .count = count
    };
}
//@source src/analyzer/diagnostic.c
#include "analyzer/diagnostic.h"
#include "analyzer/constraint_engine.h"
#include "analyzer/constraint_diagnostic.h"
#include <stdlib.h>
DiagnosticArtifact analyze_diagnostics(struct World *head)
{
    Diagnostic *buf = calloc(256, sizeof(Diagnostic));
    size_t count = 0;
    /* --- Canonical semantic path --- */
    ConstraintArtifact constraints = analyze_constraints(head);
    count += constraint_to_diagnostic(
        &constraints,
        buf + count,
        256 - count
    );
    /* --- Temporary legacy path (shadowing only) --- */
    // count += analyze_shadowing(head, buf + count, 256 - count);
    return (DiagnosticArtifact){
        .items = buf,
        .count = count
    };
}
const char *diagnostic_kind_name(DiagnosticKind k)
{
    switch (k) {
    case DIAG_REDECLARATION: return "REDECLARATION";
    case DIAG_SHADOWING: return "SHADOWING";
    case DIAG_USE_BEFORE_DECLARE: return "USE_BEFORE_DECLARE";
    case DIAG_USE_AFTER_SCOPE_EXIT: return "USE_AFTER_SCOPE_EXIT";
    default: return "UNKNOWN";
    }
}
//@source src/analyzer/diagnostic_dump.c
#include "analyzer/diagnostic.h"
#include "analyzer/diagnostic_id.h"
#include <stdio.h>
static const char *kind_str(DiagnosticKind k)
{
    switch (k) {
    case DIAG_REDECLARATION:        return "REDECLARATION";
    case DIAG_SHADOWING:            return "SHADOWING";
    case DIAG_USE_BEFORE_DECLARE:   return "USE_BEFORE_DECLARE";
    case DIAG_USE_AFTER_SCOPE_EXIT: return "USE_AFTER_SCOPE_EXIT";
    default:                        return "UNKNOWN";
    }
}
void diagnostic_dump(const DiagnosticArtifact *a)
{
    if (!a || a->count == 0)
        return;
    printf("\n-- DIAGNOSTICS --\n");
    for (size_t i = 0; i < a->count; i++) {
        const Diagnostic *d = &a->items[i];
        printf(
            "id=%016llx time=%llu %s scope=%llu prev_scope=%llu\n",
            (unsigned long long)d->id.value,
            (unsigned long long)d->time,
            kind_str(d->kind),
            (unsigned long long)d->scope_id,
            (unsigned long long)d->prev_scope
        );
        if (d->anchor) {
            printf(
                " at node=%u line=%u col=%u\n",
                d->anchor->node_id,
                d->anchor->line,
                d->anchor->col
            );
        }
    }
}
//@source src/analyzer/diagnostic_id.c
#include <stdio.h>
#include <inttypes.h>
#include "analyzer/diagnostic_id.h"
#include "analyzer/constraint.h"
/*
 * diagnostic_id_from_constraint
 *
 * Stable semantic identity for a diagnostic.
 */
DiagnosticId diagnostic_id_from_constraint(const Constraint *c)
{
    DiagnosticId id = {0};
    if (!c)
        return id;
    /* Simple structural hash — stable across runs */
    id.value ^= (uint64_t)c->kind;
    id.value ^= (uint64_t)c->time << 16;
    id.value ^= (uint64_t)c->scope_id << 32;
    return id;
}
void diagnostic_id_render(DiagnosticId id)
{
    printf("%016" PRIx64, id.value);
}
//@source src/analyzer/diagnostic_project.c
#include "analyzer/diagnostic_project.h"
#include "analyzer/diagnostic.h"
void diagnostic_project_ndjson(
    const DiagnosticArtifact *a,
    FILE *out
)
{
    for (size_t i = 0; i < a->count; i++) {
        const Diagnostic *d = &a->items[i];
        fprintf(
            out,
            "{"
            "\"id\":\"%016llx\","
            "\"time\":%llu,"
            "\"kind\":\"%s\","
            "\"scope\":%llu,"
            "\"prev_scope\":%llu",
            (unsigned long long)d->id.value,
            (unsigned long long)d->time,
            diagnostic_kind_name(d->kind),
            (unsigned long long)d->scope_id,
            (unsigned long long)d->prev_scope
        );
        if (d->anchor) {
            fprintf(
                out,
                ",\"anchor\":{"
                "\"node\":%u,"
                "\"line\":%u,"
                "\"col\":%u"
                "}",
                d->anchor->node_id,
                d->anchor->line,
                d->anchor->col
            );
        }
        fprintf(out, "}\n");
    }
}
//@source src/analyzer/diagnostic_serialize.c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "analyzer/diagnostic.h"
#include "analyzer/diagnostic_serialize.h"
int diagnostic_deserialize_line(FILE *in, Diagnostic *out)
{
    unsigned long long id;
    unsigned kind;
    unsigned long long time;
    unsigned long long scope;
    unsigned long long prev_scope;
    int n = fscanf(
        in,
        "{"
        "\"id\":\"%llx\","
        "\"kind\":%u,"
        "\"time\":%llu,"
        "\"scope\":%llu,"
        "\"prev_scope\":%llu"
        "}",
        &id,
        &kind,
        &time,
        &scope,
        &prev_scope
    );
    if (n != 5)
        return 0;
    /* consume remainder of line */
    int c;
    while ((c = fgetc(in)) != '\n' && c != EOF) {}
    out->id.value     = (uint64_t)id;
    out->kind         = (DiagnosticKind)kind;
    out->time         = (uint64_t)time;
    out->scope_id     = (uint64_t)scope;
    out->prev_scope   = (uint64_t)prev_scope;
    out->anchor       = NULL; /* reconstructed later */
    return 1;
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
//@source src/consumers/cause_key.c
#include "cause_key.h"
inline int cause_key_equal(const CauseKey *a, const CauseKey *b)
{
    return a->step == b->step &&
           a->ast_id == b->ast_id &&
           a->scope_id == b->scope_id;
}
//@source src/consumers/cause_key_extract.c
#include "consumers/cause_key.h"
#include "consumers/root_chain.h"
#include "executor/step.h"
#include "consumers/cause_key_extract.h"
/*
 * Extract a CauseKey from a RootChain.
 *
 * Stage 7 discipline:
 * - deterministic
 * - no allocation
 * - no mutation
 *
 * Strategy:
 * - Use the FIRST node in the chain (closest causal event)
 */
int extract_cause_key(
    const RootChain *chain,
    CauseKey *out
)
{
    if (!chain || !out || chain->count == 0) {
        return 1;
    }
    const RootChainNode *n = &chain->nodes[0];
    out->step     = n->step;
    out->ast_id   = n->ast_id;
    out->scope_id = n->scope_id;
    return 0;
}
//@source src/consumers/convergence_build.c
#include <stddef.h>
#include "consumers/convergence_map.h"
#include "consumers/root_chain.h"
#include "consumers/cause_key_extract.h"  
#include "consumers/cause_key.h"
#include "analyzer/diagnostic.h"
/*
 * Build convergence groups across diagnostics by causal signature.
 */
int build_convergence_map(
    const DiagnosticArtifact *diags,
    const RootChain *chains,
    ConvergenceMap *out
)
{
    if (!diags || !chains || !out)
        return -1;
    for (size_t i = 0; i < diags->count; i++) {
        CauseKey key;
        if (extract_cause_key(&chains[i], &key) == 0) {
            convergence_map_add(out, &key, &diags->items[i]);
        }
    }
    return 0;
}
//@source src/consumers/convergence_map.c
#include <stdlib.h>
#include <string.h>
#include "consumers/convergence_map.h"
#include "consumers/cause_key.h"
/* Forward declaration — real definition lives in cause_key.c */
int cause_key_equal(const CauseKey *a, const CauseKey *b);
static ConvergenceEntry *
find_entry(ConvergenceMap *m, const CauseKey *key)
{
    for (size_t i = 0; i < m->count; i++) {
        if (cause_key_equal(&m->entries[i].key, key))
            return &m->entries[i];
    }
    return NULL;
}
void convergence_map_add(
    ConvergenceMap *m,
    const CauseKey *key,
    const Diagnostic *diag
)
{
    ConvergenceEntry *e = find_entry(m, key);
    if (!e) {
        if (m->count == m->capacity) {
            size_t nc = m->capacity ? m->capacity * 2 : 4;
            m->entries = realloc(m->entries, nc * sizeof(*m->entries));
            m->capacity = nc;
        }
        e = &m->entries[m->count++];
        memset(e, 0, sizeof(*e));
        e->key = *key;
    }
    if (e->count == e->capacity) {
        size_t nc = e->capacity ? e->capacity * 2 : 4;
        e->diagnostics = realloc(e->diagnostics, nc * sizeof(*e->diagnostics));
        e->capacity = nc;
    }
    e->diagnostics[e->count++] = diag;
}
//@source src/consumers/convergence_render.c
#include <stdio.h>
#include "consumers/convergence_map.h"
#include "executor/step.h"
#include "analyzer/diagnostic.h"
/*
 * render_convergence
 *
 * Presentation-only.
 * No building, no allocation, no mutation.
 */
void render_convergence(const ConvergenceMap *m)
{
    if (!m || m->count == 0)
        return;
    printf("\n== Cross-Diagnostic Convergence ==\n");
    for (size_t i = 0; i < m->count; i++) {
        const ConvergenceEntry *e = &m->entries[i];
        /* Only interesting if multiple diagnostics converge */
        if (e->count < 2)
            continue;
        printf(
            "\nCAUSE: step=%s ast=%llu scope=%llu\n",
            step_kind_name(e->key.step),
            (unsigned long long)e->key.ast_id,
            (unsigned long long)e->key.scope_id
        );
        for (size_t j = 0; j < e->count; j++) {
            const Diagnostic *d = e->diagnostics[j];
            printf(
                "  ↳ %s (time=%llu)\n",
                diagnostic_kind_name(d->kind),
                (unsigned long long)d->time
            );
        }
    }
}
//@source src/consumers/diagnostic_anchor.c
#include "consumers/diagnostic_anchor.h"
int anchor_diagnostic(
    const Diagnostic *d,
    const TimelineEvent *events,
    size_t event_count,
    DiagnosticAnchor *out
)
{
    if (!d || !events || !out)
        return 0;
    TimelineEvent best = {0};
    int found = 0;
    for (size_t i = 0; i < event_count; i++) {
        if (events[i].time <= d->time) {
            best = events[i];
            found = 1;
        } else {
            break;
        }
    }
    if (!found)
        return 0;
    *out = (DiagnosticAnchor){
        .id = d->id,
        .diagnostic_time = d->time,
        .cause = best
    };
    return 1;
}
//@source src/consumers/diagnostic_diff.c
#include "consumers/diagnostic_diff.h"
/*
 * diagnostic_diff
 *
 * Compare two diagnostic artifacts by stable DiagnosticId.
 *
 * Output guarantees:
 *  - Each DiagnosticId appears at most once
 *  - No allocation
 *  - No sorting
 *  - Deterministic
 *
 * Complexity: O(n²) by design (small n, stable identity)
 */
size_t diagnostic_diff(
    const DiagnosticArtifact *old_run,
    const DiagnosticArtifact *new_run,
    DiagnosticDiff *out,
    size_t cap
)
{
    size_t count = 0;
    if (!out || cap == 0)
        return 0;
    /* ---- REMOVED / UNCHANGED ---- */
    for (size_t i = 0; old_run && i < old_run->count; i++) {
        DiagnosticId id = old_run->items[i].id;
        int found = 0;
        for (size_t j = 0; new_run && j < new_run->count; j++) {
            if (new_run->items[j].id.value == id.value) {
                found = 1;
                break;
            }
        }
        if (count >= cap)
            return count;
        out[count++] = (DiagnosticDiff){
            .kind = found ? DIFF_UNCHANGED : DIFF_REMOVED,
            .id   = id
        };
    }
    /* ---- ADDED ---- */
    for (size_t i = 0; new_run && i < new_run->count; i++) {
        DiagnosticId id = new_run->items[i].id;
        int found = 0;
        for (size_t j = 0; old_run && j < old_run->count; j++) {
            if (old_run->items[j].id.value == id.value) {
                found = 1;
                break;
            }
        }
        if (!found && count < cap) {
            out[count++] = (DiagnosticDiff){
                .kind = DIFF_ADDED,
                .id   = id
            };
        }
    }
    return count;
}
//@source src/consumers/diagnostic_render.c
#include "consumers/diagnostic_render.h"
#include <stdio.h>
#include "analyzer/diagnostic.h"
/*
 * Terminal renderer for diagnostics.
 *
 * Stage 5.4 discipline:
 *  - read-only
 *  - no allocation
 *  - no formatting state
 *  - stable output
 */
void
diagnostic_render_terminal(
    const DiagnosticArtifact *a,
    FILE *out
) {
    if (!a || !a->items || a->count == 0) {
        fprintf(out, "(no diagnostics)\n");
        return;
    }
    for (size_t i = 0; i < a->count; i++) {
        const Diagnostic *d = &a->items[i];
        fprintf(
            out,
            "[%02zu] %-24s time=%llu scope=%llu",
            i,
            diagnostic_kind_name(d->kind),
            (unsigned long long)d->time,
            (unsigned long long)d->scope_id
        );
        if (d->prev_scope) {
            fprintf(
                out,
                " prev_scope=%llu",
                (unsigned long long)d->prev_scope
            );
        }
        fprintf(
            out,
            " id=%016llx",
            (unsigned long long)d->id.value
        );
        if (d->anchor) {
            fprintf(
                out,
                " @ node=%u:%u:%u",
                d->anchor->node_id,
                d->anchor->line,
                d->anchor->col
            );
        }
        fputc('\n', out);
    }
}
//@source src/consumers/diagnostic_stats.c
#include "consumers/diagnostic_stats.h"
#include <string.h>
void diagnostic_stats_compute(
    const DiagnosticArtifact *a,
    DiagnosticStats *out
)
{
    if (!a || !out)
        return;
    memset(out, 0, sizeof(*out));
    out->total = a->count;
    for (size_t i = 0; i < a->count; i++) {
        DiagnosticKind k = a->items[i].kind;
        if ((size_t)k < sizeof(out->by_kind) / sizeof(out->by_kind[0])) {
            out->by_kind[k]++;
        }
    }
}
//@source src/consumers/diagnostic_stats_emit.c
#include <stdio.h>
#include "consumers/diagnostic_stats.h"
void diagnostic_stats_emit(
    const DiagnosticStats *stats,
    FILE *out
)
{
    fprintf(out, "total: %zu\n", stats->total);
    for (size_t i = 0; i < DIAG_KIND_MAX; i++) {
        if (stats->by_kind[i]) {
            fprintf(out, "  kind[%zu]: %zu\n",
                    i, stats->by_kind[i]);
        }
    }
}
//@source src/consumers/diagnostic_timeline.c
#include <stdio.h>
#include "analyzer/diagnostic.h"
#include "analyzer/diagnostic_serialize.h"
void diagnostic_timeline_render(FILE *in, FILE *out)
{
    Diagnostic d;
    fprintf(out,
        "%-5s %-24s %-5s %s\n",
        "TIME", "KIND", "SCOPE", "ID"
    );
    fprintf(out,
        "%-5s %-24s %-5s %s\n",
        "----", "-----------------------", "-----",
        "----------------"
    );
    while (diagnostic_deserialize_line(in, &d)) {
        fprintf(
            out,
            "%5llu %-24s %5llu %016llx\n",
            (unsigned long long)d.time,
            diagnostic_kind_name(d.kind),
            (unsigned long long)d.scope_id,
            (unsigned long long)d.id.value
        );
    }
}
//@source src/consumers/diagnostic_validate.c
#include "consumers/diagnostic_validate.h"
/*
 * validate_diagnostics
 *
 * Structural gate over DiagnosticArtifact.
 *
 * Detects:
 *   - duplicate DiagnosticId
 *   - non-monotonic time
 *
 * Discipline:
 *   - no allocation
 *   - no mutation
 *   - deterministic
 */
size_t validate_diagnostics(
    const DiagnosticArtifact *a,
    ValidationIssue *out,
    size_t cap
)
{
    size_t count = 0;
    if (!a || !out || cap == 0)
        return 0;
    /* ---- Duplicate IDs ---- */
    for (size_t i = 0; i < a->count; i++) {
        for (size_t j = i + 1; j < a->count; j++) {
            if (a->items[i].id.value == a->items[j].id.value) {
                if (count < cap) {
                    out[count++] = (ValidationIssue){
                        .kind = VALIDATION_DUPLICATE_ID,
                        .id   = a->items[i].id
                    };
                }
            }
        }
    }
    /* ---- Monotonic time ---- */
    for (size_t i = 1; i < a->count; i++) {
        if (a->items[i].time < a->items[i - 1].time) {
            if (count < cap) {
                out[count++] = (ValidationIssue){
                    .kind = VALIDATION_NON_MONOTONIC_TIME,
                    .id   = a->items[i].id
                };
            }
        }
    }
    return count;
}
//@source src/consumers/fix_surface_build.c
#include <stdlib.h>
#include <string.h>
#include "consumers/fix_surface.h"
#include "consumers/convergence_map.h"
static void add_cause(FixSurface *fs, const CauseKey *key)
{
    if (fs->count == fs->capacity) {
        size_t nc = fs->capacity ? fs->capacity * 2 : 4;
        fs->causes = realloc(fs->causes, nc * sizeof(*fs->causes));
        fs->capacity = nc;
    }
    fs->causes[fs->count++] = *key;
}
FixSurface build_fix_surface(const ConvergenceMap *map)
{
    FixSurface fs = {0};
    /* In current architecture:
       every diagnostic belongs to exactly one cause.
       So minimal fix surface = all unique causes.
     */
    for (size_t i = 0; i < map->count; i++) {
        add_cause(&fs, &map->entries[i].key);
    }
    return fs;
}
//@source src/consumers/fix_surface_render.c
#include <stdio.h>
#include "consumers/fix_surface.h"
#include "executor/step.h"
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
//@source src/consumers/load_diagnostics.c
#include "analyzer/diagnostic.h"
#include "analyzer/diagnostic_serialize.h"
#include <stdio.h>
#include <stdlib.h>
int load_diagnostics(const char *path, DiagnosticArtifact *out)
{
    if (!path || !out)
        return 1;
    FILE *f = fopen(path, "r");
    if (!f)
        return 2;
    Diagnostic *buf = calloc(256, sizeof(Diagnostic));
    size_t count = 0;
    while (count < 256 &&
           diagnostic_deserialize_line(f, &buf[count])) {
        count++;
    }
    fclose(f);
    out->items = buf;
    out->count = count;
    return 0;
}
//@source src/consumers/load_run.c
#include "consumers/run_descriptor.h"
#include "consumers/run_artifact.h"
int run_probe(const RunDescriptor *rd);
int load_diagnostics(const char *path, DiagnosticArtifact *out);
int load_timeline(const char *path, void **out, size_t *count);
int load_run(const RunDescriptor *rd, RunArtifact *out)
{
    if (!rd || !out)
        return 1;
    int rc = run_probe(rd);
    if (rc != 0)
        return rc;
    out->run_id = (char *)rd->run_id;
    if (load_diagnostics(rd->diagnostics_path,
                          &out->diagnostics) != 0)
        return 10;
    /* Timeline optional */
    if (rd->timeline_path) {
        load_timeline(
            rd->timeline_path,
            (void **)&out->timeline,
            &out->timeline_count
        );
    }
    return 0;
}
//@source src/consumers/load_timeline.c
#include <stdio.h>
#include <stdlib.h>
typedef struct TimelineEvent {
    unsigned long time;
    int step;
    unsigned int ast;
} TimelineEvent;
int load_timeline(const char *path,
                  TimelineEvent **out,
                  size_t *out_count)
{
    if (!path || !out || !out_count)
        return 1;
    FILE *f = fopen(path, "r");
    if (!f)
        return 2;
    TimelineEvent *buf = calloc(256, sizeof(TimelineEvent));
    size_t count = 0;
    while (count < 256) {
        TimelineEvent e;
        int n = fscanf(
            f,
            "{\"time\":%lu,\"step\":%d,\"ast\":%u}\n",
            &e.time,
            &e.step,
            &e.ast
        );
        if (n != 3)
            break;
        buf[count++] = e;
    }
    fclose(f);
    *out = buf;
    *out_count = count;
    return 0;
}
//@source src/consumers/root_cause_extract.c
#include "consumers/root_cause_extract.h"
#include "executor/step.h"
#include "frontends/c/ast.h"
RootCause root_cause_extract(
    const struct World *head,
    const struct Diagnostic *d
)
{
    const struct World *w = head;
    /* Seek to diagnostic time */
    while (w && w->time != d->time)
        w = w->next;
    /* Walk backwards */
    while (w && w->prev) {
        w = w->prev;
        if (!w->step)
            continue;
        ASTNode *n = (ASTNode *)w->step->origin;
        uint64_t ast_id = n ? n->id : 0;
        /* Declaration-related diagnostics */
        if (d->kind == DIAG_REDECLARATION ||
            d->kind == DIAG_SHADOWING) {
            if (w->step->kind == STEP_DECLARE) {
                return (RootCause){
                    .kind     = ROOT_CAUSE_DECLARATION,
                    .time     = w->time,
                    .ast_id   = ast_id,
                    .scope_id = d->scope_id   /* diagnostic-derived */
                };
            }
        }
        /* Use-related diagnostics */
        if (d->kind == DIAG_USE_BEFORE_DECLARE) {
            if (w->step->kind == STEP_USE) {
                return (RootCause){
                    .kind     = ROOT_CAUSE_USE,
                    .time     = w->time,
                    .ast_id   = ast_id,
                    .scope_id = d->scope_id
                };
            }
        }
        /* Scope entry / exit is authoritative */
        if (w->step->kind == STEP_ENTER_SCOPE ||
            w->step->kind == STEP_EXIT_SCOPE) {
            return (RootCause){
                .kind     = (w->step->kind == STEP_ENTER_SCOPE)
                              ? ROOT_CAUSE_SCOPE_ENTRY
                              : ROOT_CAUSE_SCOPE_EXIT,
                .time     = w->time,
                .ast_id   = ast_id,
                .scope_id = w->step->info   /* ← THIS is correct */
            };
        }
    }
    return (RootCause){
        .kind     = ROOT_CAUSE_UNKNOWN,
        .time     = d->time,
        .ast_id   = 0,
        .scope_id = d->scope_id
    };
}
//@source src/consumers/root_chain.c
#include "common/arena.h"
#include "consumers/root_chain.h"
#include "analyzer/diagnostic.h"   // ✅ REQUIRED
#include "executor/world.h"
#include "executor/step.h"
void build_and_render_root_chains(
    const World *world,
    const DiagnosticArtifact *diags
)
{
    if (diags->count == 0)
        return;
    Arena arena;
    arena_init(&arena, 4096);
    for (size_t i = 0; i < diags->count; i++) {
        RootChain chain = build_root_chain(
            &arena,
            world,
            &diags->items[i]
        );
        render_root_chain(&chain);
    }
    arena_destroy(&arena);
}
//@source src/consumers/root_chain_build.c
#include <stddef.h>
#include <stdint.h>
#include "common/arena.h"
#include "executor/world.h"
#include "executor/step.h"
#include "consumers/root_chain.h"
#include "consumers/root_chain_role.h"
#include "analyzer/diagnostic.h"
#include "frontends/c/ast.h"   /* for ASTNode */
/*
 * Build a root-cause chain by walking the World timeline backwards
 * from the diagnostic time.
 *
 * nodes[0] is the closest causal event to the diagnostic.
 */
RootChain build_root_chain(
    Arena *arena,
    const World *world_tail,
    const Diagnostic *diag
)
{
    RootChain chain = {0};
    chain.diagnostic_id = diag->id;
    /* ----------------------------------------
     * First pass: count causal steps
     * ---------------------------------------- */
    size_t count = 0;
    for (const World *w = world_tail; w; w = w->prev) {
        if (w->time > diag->time)
            continue;
        if (!w->step)
            continue;
        count++;
    }
    if (count == 0)
        return chain;
    /* ----------------------------------------
     * Allocate nodes
     * ---------------------------------------- */
    RootChainNode *nodes = arena_alloc(arena, count * sizeof(*nodes));
    if (!nodes)
        return chain;
    /* ----------------------------------------
     * Second pass: populate nodes
     * ---------------------------------------- */
    size_t i = 0;
    for (const World *w = world_tail; w; w = w->prev) {
        if (w->time > diag->time)
            continue;
        const Step *s = w->step;
        if (!s)
            continue;
        RootChainNode *n = &nodes[i++];
        n->time = w->time;
        n->step = s->kind;
        /* Derive AST id from origin */
        if (s->origin) {
            const struct ASTNode *ast = (const struct ASTNode *)s->origin;
            n->ast_id = ast->id;
        } else {
            n->ast_id = 0;
        }
        /* Derive scope id conservatively */
        switch (s->kind) {
            case STEP_ENTER_SCOPE:
            case STEP_EXIT_SCOPE:
                n->scope_id = s->info;
                break;
            default:
                n->scope_id = diag->scope_id;
                break;
        }
        n->role = ROOT_ROLE_WITNESS;
    }
    chain.nodes = nodes;
    chain.count = i;
    /* ----------------------------------------
     * Assign semantic roles
     * ---------------------------------------- */
    assign_root_chain_roles(&chain, diag->kind);
    return chain;
}
//@source src/consumers/root_chain_render.c
#include <stdio.h>
#include "consumers/root_chain.h"
#include "executor/step.h"
#include "analyzer/diagnostic_id.h"
static const char *role_str(RootRole r)
{
    switch (r) {
        case ROOT_ROLE_CAUSE:      return "CAUSE";
        case ROOT_ROLE_AMPLIFIER: return "AMPLIFIER";
        case ROOT_ROLE_WITNESS:   return "WITNESS";
        case ROOT_ROLE_SUPPRESSOR:return "SUPPRESSOR";
        default:                  return "?";
    }
}
void render_root_chain(const RootChain *chain)
{
    printf("\nROOT CAUSE CHAIN (diag=");
    diagnostic_id_render(chain->diagnostic_id);
    printf(")\n");
    printf("----------------------------\n");
    for (size_t i = 0; i < chain->count; i++) {
        const RootChainNode *n = &chain->nodes[i];
        printf(
            "[t=%llu] %-11s step=%s ast=%llu scope=%llu\n",
            (unsigned long long)n->time,
            role_str(n->role),
            step_kind_str(n->step),
            (unsigned long long)n->ast_id,
            (unsigned long long)n->scope_id
        );
    }
}
//@source src/consumers/root_chain_role.c
#include "consumers/root_chain.h"
#include "analyzer/diagnostic.h"
/*
 * Assign semantic roles based on diagnostic kind and causal distance.
 *
 * nodes[0] is the closest cause to the diagnostic.
 */
void assign_root_chain_roles(RootChain *chain, DiagnosticKind kind)
{
    if (!chain || chain->count == 0)
        return;
    for (size_t i = 0; i < chain->count; i++) {
        RootChainNode *n = &chain->nodes[i];
        if (i == 0) {
            /* Closest event to diagnostic */
            n->role = ROOT_ROLE_CAUSE;
            continue;
        }
        switch (kind) {
            case DIAG_USE_BEFORE_DECLARE:
            case DIAG_REDECLARATION:
            case DIAG_SHADOWING:
                if (n->step == STEP_DECLARE ||
                    n->step == STEP_USE) {
                    n->role = ROOT_ROLE_AMPLIFIER;
                } else {
                    n->role = ROOT_ROLE_WITNESS;
                }
                break;
            default:
                n->role = ROOT_ROLE_WITNESS;
                break;
        }
    }
}
//@source src/consumers/run_probe.c
#include "consumers/run_descriptor.h"
#include "consumers/run_contract.h"
#include <sys/stat.h>
static int file_exists(const char *path)
{
    struct stat st;
    return path && stat(path, &st) == 0;
}
int run_probe(const RunDescriptor *rd)
{
    if (!rd)
        return 1;
    if (LIMINAL_RUN_CONTRACT.require_meta &&
        !file_exists(rd->meta_path))
        return 2;
    if (LIMINAL_RUN_CONTRACT.require_diagnostics &&
        !file_exists(rd->diagnostics_path))
        return 3;
    if (!LIMINAL_RUN_CONTRACT.allow_missing_timeline &&
        !file_exists(rd->timeline_path))
        return 4;
    return 0;
}
//@source src/consumers/run_validate.c
#include "consumers/run_artifact.h"
int validate_run(const RunArtifact *r)
{
    if (!r)
        return 1;
    /* diagnostics time monotonic */
    for (size_t i = 1; i < r->diagnostics.count; i++) {
        if (r->diagnostics.items[i].time <
            r->diagnostics.items[i - 1].time)
            return 2;
    }
    /* timeline monotonic */
    for (size_t i = 1; i < r->timeline_count; i++) {
        if (r->timeline[i].time <
            r->timeline[i - 1].time)
            return 3;
    }
    return 0;
}
//@source src/consumers/scope_align.c
#include "consumers/scope_alignment.h"
#include "consumers/scope_signature.h"
size_t scope_align(
    const ScopeGraph *a,
    const ScopeGraph *b,
    ScopeAlignment *out,
    size_t cap
)
{
    size_t count = 0;
    /* Removed / unchanged */
    for (size_t i = 0; i < a->count && count < cap; i++) {
        uint64_t sig_a = scope_signature(&a->nodes[i]);
        int found = 0;
        for (size_t j = 0; j < b->count; j++) {
            if (sig_a == scope_signature(&b->nodes[j])) {
                found = 1;
                break;
            }
        }
        out[count++] = (ScopeAlignment){
            .old_sig = sig_a,
            .new_sig = found ? sig_a : 0,
            .kind = found ? SCOPE_UNCHANGED : SCOPE_REMOVED
        };
    }
    /* Added */
    for (size_t j = 0; j < b->count && count < cap; j++) {
        uint64_t sig_b = scope_signature(&b->nodes[j]);
        int found = 0;
        for (size_t i = 0; i < a->count; i++) {
            if (sig_b == scope_signature(&a->nodes[i])) {
                found = 1;
                break;
            }
        }
        if (!found) {
            out[count++] = (ScopeAlignment){
                .old_sig = 0,
                .new_sig = sig_b,
                .kind = SCOPE_ADDED
            };
        }
    }
    return count;
}
//@source src/consumers/scope_align_render.c
#include <stdio.h>
#include "consumers/scope_alignment.h"
static const char *scope_kind_str(ScopeChangeKind k)
{
    switch (k) {
    case SCOPE_ADDED:    return "ADDED";
    case SCOPE_REMOVED:  return "REMOVED";
    case SCOPE_MOVED:    return "MOVED";
    case SCOPE_SPLIT:    return "SPLIT";
    case SCOPE_MERGED:   return "MERGED";
    default:             return "UNCHANGED";
    }
}
void scope_align_render(
    const ScopeAlignment *a,
    size_t count,
    FILE *out
)
{
    fprintf(out,
        "%-18s %-18s %-10s\n",
        "OLD_SIG", "NEW_SIG", "CHANGE"
    );
    for (size_t i = 0; i < count; i++) {
        fprintf(
            out,
            "%016llx %016llx %-10s\n",
            (unsigned long long)a[i].old_sig,
            (unsigned long long)a[i].new_sig,
            scope_kind_str(a[i].kind)
        );
    }
}
//@source src/consumers/scope_graph_extract.c
#include "consumers/scope_graph_extract.h"
#include "executor/step.h"
#include "executor/scope.h"  
#include <stdlib.h>
ScopeGraph scope_graph_extract(
    const struct World *head
)
{
    ScopeNode *buf = calloc(128, sizeof(ScopeNode));
    size_t count = 0;
    const struct World *w = head;
    while (w) {
        if (w->step) {
            if (w->step->kind == STEP_ENTER_SCOPE) {
                ScopeNode *n = &buf[count++];
                n->scope_id   = w->step->info;
                n->parent_id  =
                    w->prev && w->prev->active_scope
                        ? w->prev->active_scope->id
                        : 0;
                n->enter_time = w->time;
                n->exit_time  = UINT64_MAX;
            }
            if (w->step->kind == STEP_EXIT_SCOPE) {
                uint64_t sid = w->step->info;
                for (size_t i = count; i > 0; i--) {
                    if (buf[i - 1].scope_id == sid &&
                        buf[i - 1].exit_time == UINT64_MAX) {
                        buf[i - 1].exit_time = w->time;
                        break;
                    }
                }
            }
        }
        w = w->next;
    }
    return (ScopeGraph){
        .nodes = buf,
        .count = count
    };
}
//@source src/consumers/scope_signature.c
#include "consumers/scope_signature.h"
uint64_t scope_signature(const ScopeNode *n)
{
    /*
     * Stable semantic signature:
     *
     * [ parent_id | enter_time | exit_time ]
     *
     * Scope ID deliberately excluded.
     */
    return
        (n->parent_id << 48) ^
        (n->enter_time << 24) ^
        (n->exit_time);
}
//@source src/consumers/semantic_cause_diff.c
#include "consumers/semantic_cause_diff.h"
CauseChangeKind classify_cause(
    const DiagnosticAnchor *a,
    const DiagnosticAnchor *b,
    uint32_t *old_step,
    uint32_t *new_step
)
{
    if (!a && b) {
        *old_step = 0;
        *new_step = b->cause.step_kind;
        return CAUSE_NEW_STEP;
    }
    if (a && !b) {
        *old_step = a->cause.step_kind;
        *new_step = 0;
        return CAUSE_STEP_REMOVED;
    }
    if (a && b && a->cause.step_kind != b->cause.step_kind) {
        *old_step = a->cause.step_kind;
        *new_step = b->cause.step_kind;
        return CAUSE_STEP_MOVED;
    }
    return CAUSE_NONE;
}
//@source src/consumers/semantic_cause_render.c
#include <stdio.h>
#include "consumers/semantic_cause_diff.h"
static const char *cause_str(CauseChangeKind k)
{
    switch (k) {
    case CAUSE_NEW_STEP:     return "NEW_STEP";
    case CAUSE_STEP_REMOVED: return "REMOVED_STEP";
    case CAUSE_STEP_MOVED:   return "STEP_CHANGED";
    default:                 return "UNCHANGED";
    }
}
void semantic_cause_render(
    const SemanticCauseDiff *d,
    size_t count,
    FILE *out
)
{
    fprintf(out,
        "%-10s %-18s %-10s %-10s %-12s\n",
        "DIFF", "ID", "OLD_STEP", "NEW_STEP", "CAUSE"
    );
    for (size_t i = 0; i < count; i++) {
        fprintf(
            out,
            "%-10d %016llx %-10u %-10u %-12s\n",
            d[i].base.kind,
            (unsigned long long)d[i].base.id.value,
            d[i].old_step,
            d[i].new_step,
            cause_str(d[i].cause)
        );
    }
}
//@source src/consumers/semantic_diff.c
#include "consumers/semantic_diff.h"
/*
 * semantic_diff
 *
 * Identity axis: DiagnosticId
 * Temporal axis: time
 */
size_t semantic_diff(
    const DiagnosticArtifact *old_run,
    const DiagnosticArtifact *new_run,
    SemanticDiff *out,
    size_t cap
)
{
    size_t count = 0;
    if (!out || cap == 0)
        return 0;
    /* ---- REMOVED / UNCHANGED / MOVED ---- */
    for (size_t i = 0; old_run && i < old_run->count; i++) {
        const Diagnostic *d_old = &old_run->items[i];
        int found = 0;
        for (size_t j = 0; new_run && j < new_run->count; j++) {
            const Diagnostic *d_new = &new_run->items[j];
            if (d_new->id.value == d_old->id.value) {
                found = 1;
                if (count >= cap)
                    return count;
                if (d_new->time == d_old->time) {
                    out[count++] = (SemanticDiff){
                        .kind = SEMDIFF_UNCHANGED,
                        .id = d_old->id,
                        .old_time = d_old->time,
                        .new_time = d_new->time
                    };
                } else {
                    out[count++] = (SemanticDiff){
                        .kind = SEMDIFF_MOVED,
                        .id = d_old->id,
                        .old_time = d_old->time,
                        .new_time = d_new->time
                    };
                }
                break;
            }
        }
        if (!found && count < cap) {
            out[count++] = (SemanticDiff){
                .kind = SEMDIFF_REMOVED,
                .id = d_old->id,
                .old_time = d_old->time,
                .new_time = 0
            };
        }
    }
    /* ---- ADDED ---- */
    for (size_t i = 0; new_run && i < new_run->count; i++) {
        const Diagnostic *d_new = &new_run->items[i];
        int found = 0;
        for (size_t j = 0; old_run && j < old_run->count; j++) {
            if (old_run->items[j].id.value == d_new->id.value) {
                found = 1;
                break;
            }
        }
        if (!found && count < cap) {
            out[count++] = (SemanticDiff){
                .kind = SEMDIFF_ADDED,
                .id = d_new->id,
                .old_time = 0,
                .new_time = d_new->time
            };
        }
    }
    return count;
}
//@source src/consumers/semantic_diff_render.c
#include <stdio.h>
#include "consumers/semantic_diff.h"
static const char *kind_str(SemanticDiffKind k)
{
    switch (k) {
    case SEMDIFF_ADDED:     return "ADDED";
    case SEMDIFF_REMOVED:   return "REMOVED";
    case SEMDIFF_MOVED:     return "MOVED";
    case SEMDIFF_UNCHANGED: return "UNCHANGED";
    default:                return "UNKNOWN";
    }
}
void semantic_diff_render(
    const SemanticDiff *diffs,
    size_t count,
    FILE *out
)
{
    fprintf(out,
        "%-10s %-18s %-10s %-10s\n",
        "KIND", "ID", "OLD", "NEW"
    );
    fprintf(out,
        "%-10s %-18s %-10s %-10s\n",
        "----------",
        "------------------",
        "----------",
        "----------"
    );
    for (size_t i = 0; i < count; i++) {
        fprintf(
            out,
            "%-10s %016llx %-10llu %-10llu\n",
            kind_str(diffs[i].kind),
            (unsigned long long)diffs[i].id.value,
            (unsigned long long)diffs[i].old_time,
            (unsigned long long)diffs[i].new_time
        );
    }
}
//@source src/consumers/timeline_diff.c
#include <stdio.h>
#include <string.h>
size_t timeline_diff_first_line(
    FILE *a,
    FILE *b
)
{
    char la[512];
    char lb[512];
    size_t line = 0;
    rewind(a);
    rewind(b);
    while (1) {
        char *ra = fgets(la, sizeof la, a);
        char *rb = fgets(lb, sizeof lb, b);
        if (!ra && !rb)
            return (size_t)-1; /* identical */
        if (!ra || !rb)
            return line;
        if (strcmp(la, lb) != 0)
            return line;
        line++;
    }
}
//@source src/consumers/timeline_emit.c
#include <stdio.h>
#include <stdint.h>
#include "consumers/timeline_emit.h"
#include "executor/world.h"
#include "executor/step.h"
#include "frontends/c/ast.h"
/*
 * Timeline NDJSON — Stage 7 canonical artifact
 *
 * Contract:
 *  - One line per World
 *  - Deterministic
 *  - No executor pointers
 *  - No formatting variance
 *  - Stable across runs
 *
 * Schema v1:
 * { "v":1, "t":<uint64>, "step":"<name>", "ast":<uint32> }
 */
void timeline_emit_ndjson(
    const struct World *head,
    FILE *out
)
{
    const struct World *w = head;
    while (w) {
        uint32_t ast_id = 0;
        const char *step_name = "UNKNOWN";
        if (w->step) {
            step_name = step_kind_name(w->step->kind);
            if (w->step->origin) {
                const ASTNode *n =
                    (const ASTNode *)w->step->origin;
                ast_id = n->id;
            }
        }
        fprintf(
            out,
            "{\"v\":1,\"t\":%llu,\"step\":\"%s\",\"ast\":%u}\n",
            (unsigned long long)w->time,
            step_name,
            ast_id
        );
        w = w->next;
    }
}
/*
 * Human-readable timeline (NON-CANONICAL)
 *
 * Debug / inspection only.
 * NOT used for diffing or artifacts.
 */
void emit_timeline(
    const struct World *head,
    FILE *out
)
{
    const struct World *w = head;
    while (w) {
        uint32_t ast_id = 0;
        if (w->step && w->step->origin) {
            const ASTNode *n = (const ASTNode *)w->step->origin;
            ast_id = n->id;
        }
        fprintf(
            out,
            "t=%llu step=%d ast=%u\n",
            (unsigned long long)w->time,
            w->step ? w->step->kind : 0,
            ast_id
        );
        w = w->next;
    }
}
//@source src/consumers/timeline_extract.c
#include "consumers/timeline_extract.h"
#include "executor/step.h"
#include "frontends/c/ast.h"
size_t timeline_extract(
    const struct World *head,
    TimelineEvent *out,
    size_t cap
)
{
    size_t count = 0;
    const struct World *w = head;
    while (w && count < cap) {
        uint32_t ast_id = 0;
        if (w->step && w->step->origin) {
            const ASTNode *n = (const ASTNode *)w->step->origin;
            ast_id = n->id;
        }
        out[count++] = (TimelineEvent){
            .time = w->time,
            .step_kind = w->step ? w->step->kind : 0,
            .ast_id = ast_id
        };
        w = w->next;
    }
    return count;
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
//@source src/commands/cmd_analyze.c
#include <stdio.h>
#include <stdlib.h>
#include "common/arena.h"
#include "executor/world.h"
#include "analyzer/diagnostic.h"
#include "consumers/root_chain.h"
#include "consumers/root_chain_role.h"
#include "consumers/convergence_map.h"
#include "consumers/fix_surface.h"
/* Forward declarations for render/build functions
 * (because you only have .c files, no headers)
 */
void render_convergence(const ConvergenceMap *m);
void render_fix_surface(const FixSurface *fs);
int build_convergence_map(
    const DiagnosticArtifact *diags,
    const RootChain *chains,
    ConvergenceMap *out
);
FixSurface build_fix_surface(const ConvergenceMap *map);
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
int cmd_analyze(const World *world)
{
    if (!world) {
        fprintf(stderr, "analyze: no world provided\n");
        return 1;
    }
    /* --- Diagnostics --- */
    DiagnosticArtifact diags =
        analyze_diagnostics((World *)world);
    if (diags.count == 0) {
        printf("No diagnostics.\n");
        return 0;
    }
    /* --- Arena for derived artifacts --- */
    Arena arena;
    arena_init(&arena, 32 * 1024);
    /* --- Root chains --- */
    RootChain *chains =
        arena_alloc(&arena, diags.count * sizeof(RootChain));
    for (size_t i = 0; i < diags.count; i++) {
        chains[i] = build_root_chain(
            &arena,
            world,
            &diags.items[i]
        );
        assign_root_chain_roles(
            &chains[i],
            diags.items[i].kind
        );
    }
    /* --- Convergence --- */
    ConvergenceMap cmap = {0};
    build_convergence_map(
        &diags,
        chains,
        &cmap
    );
    render_convergence(&cmap);
    /* --- Minimal fix surface --- */
    FixSurface fs = build_fix_surface(&cmap);
    render_fix_surface(&fs);
    arena_destroy(&arena);
    return 0;
}
//@source src/commands/cmd_diff.c
#include <stdio.h>
#include <string.h>
#include "consumers/run_descriptor.h"
#include "consumers/run_artifact.h"
#include "consumers/semantic_diff.h"
#include "consumers/timeline_diff.h"
int load_run(const RunDescriptor *, RunArtifact *);
void semantic_diff_render(
    const SemanticDiff *, size_t, FILE *
);
int cmd_diff(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr,
            "usage: liminal diff <run-A> <run-B>\n");
        return 1;
    }
    RunDescriptor a = {
        .root_dir = argv[0],
        .run_id = argv[0],
        .meta_path = ".liminal/meta.json",
        .diagnostics_path = ".liminal/diagnostics.ndjson",
        .timeline_path = ".liminal/timeline.ndjson"
    };
    RunDescriptor b = {
        .root_dir = argv[1],
        .run_id = argv[1],
        .meta_path = ".liminal/meta.json",
        .diagnostics_path = ".liminal/diagnostics.ndjson",
        .timeline_path = ".liminal/timeline.ndjson"
    };
    RunArtifact ra = {0};
    RunArtifact rb = {0};
    if (load_run(&a, &ra) != 0 ||
        load_run(&b, &rb) != 0) {
        fprintf(stderr, "failed to load runs\n");
        return 1;
    }
    SemanticDiff diffs[256];
    size_t n = semantic_diff(
        &ra.diagnostics,
        &rb.diagnostics,
        diffs,
        256
    );
    semantic_diff_render(diffs, n, stdout);
    FILE *ta = fopen(a.timeline_path, "r");
    FILE *tb = fopen(b.timeline_path, "r");
    if (ta && tb) {
        size_t d = timeline_diff_first_line(ta, tb);
        if (d != (size_t)-1) {
            printf("TIMELINE DIVERGENCE at line %zu\n", d);
        }
    }
    return 0;
}
//@source src/commands/cmd_policy.c
#include <stdio.h>
#include "commands/cmd_policy.h"
#include "policy/policy.h"
int
cmd_apply_policy(
    const Policy *policy,
    const DiagnosticArtifact *diagnostics
)
{
    PolicyDecision d = policy_evaluate(policy, diagnostics);
    switch (d) {
    case POLICY_ALLOW:
        return 0;
    case POLICY_WARN:
        fprintf(stderr, "policy warning\n");
        return 0;
    case POLICY_DENY:
        fprintf(stderr, "policy denied execution\n");
        return 1;
    }
    return 0;
}
//@source src/commands/command_dispatch.c
#include <stdio.h>
#include <string.h>
#include "command.h"
int dispatch_command(
    int argc,
    char **argv,
    const CommandSpec *commands,
    int command_count
) {
    if (argc < 1) {
        fprintf(stderr, "error: missing command\n");
        return 1;
    }
    const char *cmd = argv[0];
    for (int i = 0; i < command_count; i++) {
        if (strcmp(cmd, commands[i].name) == 0) {
            if (argc - 1 < commands[i].min_args) {
                fprintf(stderr, "error: missing arguments for '%s'\n", cmd);
                return 1;
            }
            return commands[i].handler(argc - 1, argv + 1);
        }
    }
    fprintf(stderr, "error: unknown command '%s'\n", cmd);
    return 1;
}
//@source src/policy/default_policy.c
#include "policy/default_policy.h"
#include "policy/policy.h"
static const PolicyRule DEFAULT_RULES[] = {
    /* Hard denies */
    { DIAG_USE_BEFORE_DECLARE, 0, 1 },
    { DIAG_REDECLARATION,      0, 1 },
    /* Soft limits */
    { DIAG_SHADOWING,         16, 0 }
};
const Policy LIMINAL_DEFAULT_POLICY = {
    .deny_kind = {
        [DIAG_USE_BEFORE_DECLARE] = 1,
        [DIAG_REDECLARATION]      = 1
    },
    .max_by_kind = {
        [DIAG_SHADOWING] = 16
    },
    .max_total = 64
};
//@source src/policy/policy.c
#include "policy/policy.h"
PolicyDecision
policy_evaluate(
    const Policy *policy,
    const DiagnosticArtifact *diagnostics
)
{
    if (!policy || !diagnostics) {
        return POLICY_ALLOW;
    }
    size_t by_kind[DIAG_KIND_MAX] = {0};
    for (size_t i = 0; i < diagnostics->count; i++) {
        DiagnosticKind k = diagnostics->items[i].kind;
        /* Deny rule */
        if (policy->deny_kind[k]) {
            return POLICY_DENY;
        }
        by_kind[k]++;
        /* Per-kind cap */
        if (policy->max_by_kind[k] &&
            by_kind[k] > policy->max_by_kind[k]) {
            return POLICY_DENY;
        }
    }
    /* Total cap */
    if (policy->max_total &&
        diagnostics->count > policy->max_total) {
        return POLICY_DENY;
    }
    return POLICY_ALLOW;
}
//@source src/policy/policy_default.c
#include "policy/policy.h"
#include <string.h>
Policy policy_default(void)
{
    Policy p;
    memset(&p, 0, sizeof(p));
    /* hard errors */
    p.deny_kind[DIAG_USE_BEFORE_DECLARE] = 1;
    p.deny_kind[DIAG_REDECLARATION]      = 1;
    /* warnings allowed but capped */
    p.max_by_kind[DIAG_SHADOWING] = 16;
    /* global budget */
    p.max_total = 64;
    return p;
}
//@source src/main.c
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "executor/universe.h"
#include "executor/world.h"
#include "executor/step.h"
#include "executor/scope.h"
#include "executor/executor.h"
#include "analyzer/trace.h"
#include "analyzer/use.h"
#include "analyzer/lifetime.h"
#include "analyzer/diagnostic.h"
#include "analyzer/constraint_engine.h"
#include "analyzer/artifact_emit.h"
#include "frontends/c/ast.h"
#include "frontends/c/frontend.h"
#include "commands/command.h"
#include "commands/cmd_analyze.h"
#include "commands/cmd_policy.h"
#include "commands/cmd_diff.h"
#include "consumers/timeline_emit.h"
#include "policy/default_policy.h"
/*
 * Liminal CLI entry point
 *
 * Orchestration ONLY.
 * No semantics.
 * No execution logic.
 * No analysis logic.
 */
static void print_usage(const char *prog)
{
    printf("Usage: %s run <file> [options]\n", prog);
    printf("\nOptions:\n");
    printf("  --emit-artifacts\n");
    printf("  --emit-timeline\n");
    printf("  --artifact-dir <path>   (default: .liminal)\n");
    printf("  --run-id <string>       (optional override)\n");
    printf("\n");
}
static int cmd_run(int argc, char **argv)
{
    const char *input_path     = NULL;
    const char *artifact_root  = ".liminal";
    const char *run_id_override = NULL;
    bool emit_artifacts = false;
    bool emit_timeline_flag = false;
    /* ---- ARG PARSING ---- */
    for (int i = 0; i < argc; i++) {
        if (!input_path && argv[i][0] != '-') {
            input_path = argv[i];
            continue;
        }
        if (strcmp(argv[i], "--emit-artifacts") == 0) {
            emit_artifacts = true;
            continue;
        }
        if (strcmp(argv[i], "--emit-timeline") == 0) {
            emit_timeline_flag = true;
            continue;
        }
        if (strcmp(argv[i], "--artifact-dir") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: --artifact-dir requires a path\n");
                return 1;
            }
            artifact_root = argv[++i];
            continue;
        }
        if (strcmp(argv[i], "--run-id") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: --run-id requires value\n");
                return 1;
            }
            run_id_override = argv[++i];
            continue;
        }
    }
    if (!input_path) {
        fprintf(stderr, "error: no input file\n");
        return 1;
    }
    /* ---- FRONTEND ---- */
    ASTProgram *ast = c_parse_file_to_ast(input_path);
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
    /* ---- ANALYSIS ---- */
    DiagnosticArtifact diagnostics = analyze_diagnostics(u->head);
    diagnostic_dump(&diagnostics);
    /* ---- POLICY (STAGE 6) ---- */
    if (cmd_apply_policy(&LIMINAL_DEFAULT_POLICY, &diagnostics) != 0) {
        ast_program_free(ast);
        return 1;
    }
    /* ---- ARTIFACT EMISSION ---- */
    if (emit_artifacts || emit_timeline_flag) {
        time_t now = time(NULL);
        char run_id[64];
        if (run_id_override) {
            snprintf(run_id, sizeof(run_id), "%s", run_id_override);
        } else {
            snprintf(run_id, sizeof(run_id), "run-%lu", (unsigned long)now);
        }
        ArtifactContext ctx = {
            .root       = artifact_root,
            .run_id     = run_id,
            .input_path = input_path,
            .started_at = (unsigned long)now,
            .world_head = u->head
        };
        if (emit_artifacts) {
            artifact_emit_all(&ctx, &diagnostics);
        }
        if (emit_timeline_flag) {
          emit_timeline(u->head, stdout);
        }
    }
    ast_program_free(ast);
    return 0;
}
static const CommandSpec COMMANDS[] = {
    { "run",     0, cmd_run     },
    { "analyze", 1, cmd_analyze },
    { "diff",    2, cmd_diff    },
};
int main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 0;
    }
    return dispatch_command(
        argc - 1,
        argv + 1,
        COMMANDS,
        sizeof(COMMANDS) / sizeof(COMMANDS[0])
    );
}
