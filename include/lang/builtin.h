//
// Created by matthew on 23/10/2020.
//

#ifndef FUNCTIONAL_BUILTIN_H
#define FUNCTIONAL_BUILTIN_H

#include "constant.h"

// Typedefs from other files
typedef struct expr expr_t;
typedef struct call_stack call_stack_t;
typedef struct expr_manager expr_manager_t;
typedef struct context_builder context_builder_t;

typedef expr_t *(*builtin_function_t)(expr_manager_t *, call_stack_t *);

// Builtin population
void populate_builtins(context_builder_t *builder, expr_manager_t *expr_manager);

static void populate_numeric_binary_builtin(context_builder_t *builder, expr_manager_t *expr_manager,
                                            type_t *number_type, builtin_function_t numeric_binary_op,
                                            const char *name, size_t name_length);

// Print functions

void print_number_type(value_t *number_value);

// Builtin functions

static expr_t *builtin_create_numeric_binary_op(expr_manager_t *manager, call_stack_t *call_stack,
                                                int *first, int *second, int **result);

expr_t *builtin_add(expr_manager_t *manager, call_stack_t *call_stack);

expr_t *builtin_sub(expr_manager_t *manager, call_stack_t *call_stack);

expr_t *builtin_mul(expr_manager_t *manager, call_stack_t *call_stack);

expr_t *builtin_div(expr_manager_t *manager, call_stack_t *call_stack);

#endif //FUNCTIONAL_BUILTIN_H
