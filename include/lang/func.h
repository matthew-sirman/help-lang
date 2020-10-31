//
// Created by matthew on 23/10/2020.
//

#ifndef FUNCTIONAL_FUNC_H
#define FUNCTIONAL_FUNC_H

#include <stdbool.h>
#include <stdlib.h>

#include "type.h"
#include "constant.h"

/* A lambda term takes a single expression as an argument,
 * and returns a single expression (lx.e). Lambdas can be composed
 * on top of each other. A function is a lambda with a name.
 * */

// Typedefs for other files
typedef struct context context_t;
typedef size_t var_ptr_t;
typedef size_t expr_ptr_t;

typedef struct lambda lambda_t;
typedef size_t lambda_ptr_t;
//typedef struct lambda_node *lambda_list_t;
typedef struct function_builder function_builder_t;
typedef struct func func_t;
typedef size_t func_ptr_t;

struct lambda {
    var_ptr_t var;
    expr_ptr_t expr;
};

//struct lambda_node {
//    lambda_t lambda;
//    lambda_list_t tail;
//};

struct function_builder {
    lambda_ptr_t *lambdas;
    size_t n_lambdas;
    size_t capacity;
};

struct func {
    char *name;
//    // Array of ordered lambda functions for this function (i.e. lx.ly.x would have the lambda with variable
//    // "x" followed by the lambda with variable "y" stored)
//    lambda_t *lambdas;
    // The number of free variables associated with this function which will be equal to the number
    // of lambdas
    size_t n_free_variables;

    // A "function" type expression used for stack allocation
    expr_ptr_t func_expr;
    // Entry point for invoking this function
    expr_ptr_t entry_point;
};

function_builder_t *create_function_builder(void);

lambda_ptr_t builder_create_lambda(expr_manager_t *manager, function_builder_t *builder);

func_ptr_t compile_function(context_t *context, function_builder_t *builder, expr_manager_t *expr_manager,
                            expr_ptr_t body, const char *function_name, size_t name_length);

var_ptr_t get_function_variable(expr_manager_t *expr_manager, function_builder_t *builder,
                                const char *name, size_t name_length);

//void update_expression_vars(expr_t *e, lambda_t *abstractions);

void free_function_builder(function_builder_t *builder);

//void free_func(func_t *func);

#endif //FUNCTIONAL_FUNC_H
