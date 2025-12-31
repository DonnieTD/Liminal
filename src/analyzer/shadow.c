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
