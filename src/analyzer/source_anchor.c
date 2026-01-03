#include "analyzer/analyzer.h"
#include "frontends/frontends.h"   /* for ASTNode */
#include <stdlib.h>

SourceAnchor *anchor_from_origin(void *origin)
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
