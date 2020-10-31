//
// Created by matthew on 26/10/2020.
//

#include "../../include/lang/expr.h"
#include "../../include/compiler/compiler.h"

void compile_source(const char *source, size_t source_size, context_t **p_context, expr_manager_t **p_expr_manager) {
    // Initialise the expression manager
    *p_expr_manager = create_expr_manager();
    // Get a local pointer to the expression manager for ease
    expr_manager_t *expr_manager = *p_expr_manager;

    // Create a context
    *p_context = create_context();
    // Populate builtin functions and types
    populate_builtins(*p_context, expr_manager);

    // Parse the source code to create structures for the parsed functions
    parse_source(*p_context, expr_manager, source, source_size);

//    // After we are finished compiling, we make the compiled context
//    *p_context = compile_context(builder);

//    // Free the context builder
//    free_context_builder(builder);
}
