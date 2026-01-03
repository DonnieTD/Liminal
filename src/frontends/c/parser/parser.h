#ifndef LIMINAL_C_PARSER_H
#define LIMINAL_C_PARSER_H

#include "../ast/ast.h"
#include "../lexer/lexer.h"

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
