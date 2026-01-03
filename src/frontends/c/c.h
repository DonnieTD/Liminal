#ifndef LIMINAL_FRONTEND_C_H
#define LIMINAL_FRONTEND_C_H

#include "./ast/ast.h"
#include "./lexer/lexer.h"
#include "./parser/parser.h"

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
