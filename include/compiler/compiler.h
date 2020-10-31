//
// Created by matthew on 26/10/2020.
//

#ifndef HELP_LANG_COMPILER_H
#define HELP_LANG_COMPILER_H

#include <stdlib.h>

#include "parser.h"
#include "preprocessor.h"
#include "../lang/context.h"

/* TODO: Instead of building linked lists, we can instead use arraylist structures. This makes the relinking
 *   phase much simpler, and then means we can write compiled source to an object file more easily. The interface
 *   can primarily remain identical - the compiler will now, instead of mapping from linked lists to block memory,
 *   allocate a large block of memory for the entire structure, and then appoint various pointers to regions
 *   of this block for different section (i.e. functions, expressions etc)
 *   Then for the interpreter, we can create a new expression manager.
 */

void compile_source(const char *source, size_t source_size, context_t **context, expr_manager_t **expr_manager);

#endif //HELP_LANG_COMPILER_H
