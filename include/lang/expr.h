//
// Created by matthew on 23/10/2020.
//

#ifndef FUNCTIONAL_EXPR_H
#define FUNCTIONAL_EXPR_H

#include "var.h"
#include "constant.h"
#include "type.h"
#include "builtin.h"

/* An expression is defined as:
 * e ::= x | lx:t.e | e e | c
 * meaning an expression is either a variable (x), a lambda abstraction (lx:t.e),
 * an application (e e) or a constant (c). The constant expression is the extension
 * from the untyped lambda calculus.
 * */

// Typedefs for other files
typedef struct lambda lambda_t;
typedef struct func func_t;
typedef struct context_builder context_builder_t;

typedef struct expr expr_t;
typedef struct expr_manager expr_manager_t;
typedef struct expr_node *expr_list_t;

typedef enum { VARIABLE, ABSTRACTION, APPLICATION, CONSTANT, FUNCTION, BUILTIN } expr_mode_t;

struct expr {
    expr_mode_t mode;
    type_t *type;
    union {
        var_t *var;
        lambda_t *lam;
        struct {
            expr_t *f, *a;
        } app;
        constant_t *constant;
        func_t *func;
        builtin_function_t builtin;
    } e;
};

struct expr_manager {
    expr_list_t expressions;
};

struct expr_node {
    expr_t expr;
    expr_list_t tail;
};

expr_manager_t *create_expr_manager(void);

expr_t *create_expression(expr_manager_t *manager);

expr_t *create_abstraction_expression(context_builder_t *builder, expr_manager_t *manager, lambda_t *lam);

expr_t *create_function_expression(context_builder_t *builder, expr_manager_t *manager, func_t *func);

expr_t *create_application_expression(expr_manager_t *manager, expr_t *func, expr_t *arg);

expr_t *create_constant_expression(expr_manager_t *manager, constant_t *c);

expr_t *create_variable_expression(expr_manager_t *manager, var_t *var);

void set_expression_to_abstraction(context_builder_t *builder, expr_t *expr, lambda_t *lam);

void set_expression_to_application(expr_t *expr, expr_t *func, expr_t *arg);

void set_expression_to_constant(expr_t *expr, constant_t *c);

void set_expression_to_variable(expr_t *expr, var_t *var);

void set_expression_to_builtin(expr_t *expr, type_t *type, builtin_function_t builtin);

void free_expr_manager(expr_manager_t *manager);

#endif //FUNCTIONAL_EXPR_H
