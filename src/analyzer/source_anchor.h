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

SourceAnchor *anchor_from_origin(void *origin);

#endif /* LIMINAL_SOURCE_ANCHOR_H */
