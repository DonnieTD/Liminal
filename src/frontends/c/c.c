#include <stdio.h>
#include <stdlib.h>

#include "./lexer/lexer.h"
#include "./parser/parser.h"
#include "../../common/common.h" 

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
