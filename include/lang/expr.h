//
// Created by matthew on 23/10/2020.
//

#ifndef FUNCTIONAL_EXPR_H
#define FUNCTIONAL_EXPR_H

#include "../base.h"
#include "var.h"
#include "constant.h"
#include "type.h"
#include "builtin.h"
#include "func.h"

/* An expression is defined as:
 * e ::= x | lx:t.e | e e | c
 * meaning an expression is either a variable (x), a lambda abstraction (lx:t.e),
 * an application (e e) or a constant (c). The constant expression is the extension
 * from the untyped lambda calculus.
 * */

// Typedefs for other files
typedef struct context context_t;

typedef struct expr expr_t;
typedef struct expr_manager expr_manager_t;
typedef size_t expr_ptr_t;
//typedef struct expr_node *expr_list_t;

typedef enum { VARIABLE, ABSTRACTION, APPLICATION, CONSTANT, FUNCTION, BUILTIN } expr_mode_t;

struct expr {
    expr_mode_t mode;
    type_ptr_t type;
    union {
        var_ptr_t var;
        lambda_ptr_t lam;
        struct {
            expr_ptr_t f, a;
        } app;
        constant_ptr_t constant;
        func_ptr_t func;
        builtin_function_t builtin;
    } e;
};

struct expr_manager {
//    expr_list_t expressions;
    expr_t *expressions;
    size_t n_expressions;
    size_t expr_capacity;

    var_t *variables;
    size_t n_variables;
    size_t var_capacity;

    lambda_t *lambdas;
    size_t n_lambdas;
    size_t lam_capacity;

    constant_t *constants;
    size_t n_constants;
    size_t const_capacity;
};

//struct expr_node {
//    expr_t expr;
//    expr_list_t tail;
//};

expr_manager_t *create_expr_manager(void);

expr_ptr_t create_expression(expr_manager_t *manager);

static expr_t *create_direct_expression(expr_manager_t *manager);

expr_ptr_t create_abstraction_expression(context_t *context, expr_manager_t *manager, lambda_ptr_t lam);

expr_ptr_t create_function_expression(context_t *context, expr_manager_t *manager, func_ptr_t func);

expr_ptr_t create_application_expression(context_t *context, expr_manager_t *manager, expr_ptr_t func, expr_ptr_t arg);

expr_ptr_t create_constant_expression(expr_manager_t *manager, constant_ptr_t c);

expr_ptr_t create_variable_expression(expr_manager_t *manager, var_ptr_t var);

expr_ptr_t create_builtin_expression(expr_manager_t *manager, type_ptr_t type, builtin_function_t builtin);

void set_expression_to_abstraction(context_t *context, expr_manager_t *manager, expr_ptr_t expr, lambda_ptr_t lam);

void set_expression_to_application(context_t *context, expr_manager_t *manager, expr_ptr_t expr, expr_ptr_t func, expr_ptr_t arg);

void set_expression_to_constant(expr_manager_t *manager, expr_ptr_t expr, constant_ptr_t c);

void set_expression_to_variable(expr_manager_t *manager, expr_ptr_t expr, var_ptr_t var);

void set_expression_to_builtin(expr_manager_t *manager, expr_ptr_t expr, type_ptr_t type, builtin_function_t builtin);

expr_t *get_expression(expr_manager_t *manager, expr_ptr_t expr);

var_t *get_variable(expr_manager_t *manager, var_ptr_t var);

lambda_t *get_abstraction(expr_manager_t *manager, lambda_ptr_t lam);

constant_t *get_constant(expr_manager_t *manager, constant_ptr_t constant);

void free_expr_manager(expr_manager_t *manager);

#endif //FUNCTIONAL_EXPR_H
