#include <stdlib.h>
#include "executor/universe.h"
#include "executor/world.h"
#include "executor/scope.h"
#include "executor/step.h"
#include "executor/variable.h"

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

    u->next_scope_id = 1;
    
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
 * This creates a new Variable, clones the current World,
 * and links history.
 */
World *universe_declare_variable(Universe *u,
                                 const char *name,
                                 void *origin)
{
    if (!u || !u->current || !u->current->active_scope) {
        return NULL;
    }

    /* Clone world */
    World *next = world_clone(u, u->current);
    if (!next) {
        return NULL;
    }

    next->time = u->current->time + 1;

    /* Allocate variable */
    Variable *v = arena_alloc(&u->var_arena, sizeof(Variable));
    if (!v) {
        return NULL;
    }

    v->id       = u->next_var_id++;
    v->scope_id = next->active_scope->id;
    v->name     = name;

    /* Attach step */
    Step *s = arena_alloc(&u->step_arena, sizeof(Step));
    if (!s) {
        return NULL;
    }

    s->kind   = STEP_DECLARE;
    s->origin = origin;
    s->info   = v->id;   /* step.info = variable id */

    next->step = s;

    /* Link time */
    next->prev = u->current;
    u->current->next = next;

    u->current = next;
    u->tail = next;
    u->current_time = next->time;

    return next;
}