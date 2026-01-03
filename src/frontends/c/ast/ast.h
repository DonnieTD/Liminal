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
