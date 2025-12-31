#ifndef LIMINAL_AST_H
#define LIMINAL_AST_H

#include <stdint.h>

typedef enum {
    AST_UNKNOWN = 0,
    AST_FUNCTION,
    AST_STATEMENT,
    AST_CALL
} ASTKind;

typedef struct ASTNode {
    ASTKind kind;
    uint64_t id;      /* stable identity (monotonic) */
    uint64_t line;    /* optional, for later UI */
} ASTNode;

#endif /* LIMINAL_AST_H */
