//
// Created by matthew on 26/10/2020.
//

#include <stdio.h>
#include <assert.h>
#include "../../include/lang/context.h"
#include "../../include/lang/expr.h"

expr_manager_t *create_expr_manager(void) {
    expr_manager_t *manager = (expr_manager_t *) malloc(sizeof(expr_manager_t));

    manager->n_expressions = 0;
    manager->expr_capacity = 1;
    manager->expressions = (expr_t *) malloc(sizeof(expr_t));

    manager->n_variables = 0;
    manager->var_capacity = 1;
    manager->variables = (var_t *) malloc(sizeof(var_t));

    manager->n_lambdas = 0;
    manager->lam_capacity = 1;
    manager->lambdas = (lambda_t *) malloc(sizeof(lambda_t));

    manager->n_constants = 0;
    manager->const_capacity = 1;
    manager->constants = (constant_t *) malloc(sizeof(constant_t));

    return manager;
}

expr_ptr_t create_expression(expr_manager_t *manager) {
    if (manager->n_expressions + 1 > manager->expr_capacity) {
        manager->expr_capacity *= 2;
        manager->expressions = (expr_t *) realloc(manager->expressions, sizeof(expr_t) * manager->expr_capacity);
    }
    return ++manager->n_expressions;
//    expr_list_t node = (expr_list_t) malloc(sizeof(struct expr_node));
//    node->tail = manager->expressions;
//    manager->expressions = node;
//    return &node->expr;
}

static expr_t *create_direct_expression(expr_manager_t *manager) {
    if (manager->n_expressions + 1 > manager->expr_capacity) {
        manager->expr_capacity *= 2;
        manager->expressions = (expr_t *) realloc(manager->expressions, sizeof(expr_t) * manager->expr_capacity);
    }
    return &manager->expressions[manager->n_expressions++];
}

expr_ptr_t create_abstraction_expression(context_t *context, expr_manager_t *manager, lambda_ptr_t lam) {
    expr_ptr_t lam_expr = create_expression(manager);
    expr_t *p_lam_expr = get_expression(manager, lam_expr);
    p_lam_expr->mode = ABSTRACTION;
    p_lam_expr->e.lam = lam;
    lambda_t *p_lam = get_abstraction(manager, lam);
    p_lam_expr->type = context_create_function_type(context, get_variable(manager, p_lam->var)->type,
                                                    get_expression(manager, p_lam->expr)->type);
    return lam_expr;
}

expr_ptr_t create_function_expression(context_t *context, expr_manager_t *manager, func_ptr_t func) {
    expr_ptr_t func_expr = create_expression(manager);
    expr_t *p_func_expr = get_expression(manager, func_expr);
    p_func_expr->mode = FUNCTION;
    p_func_expr->e.func = func;
    p_func_expr->type = get_expression(manager, get_function(context, func)->entry_point)->type;// context_create_function_type(builder, func->lambdas[0].var.type, func->lambdas[0].expr.type);
    return func_expr;
}

expr_ptr_t create_application_expression(context_t *context, expr_manager_t *manager, expr_ptr_t func, expr_ptr_t arg) {
    expr_ptr_t app_expr = create_expression(manager);
    expr_t *p_app_expr = get_expression(manager, app_expr);
    p_app_expr->mode = APPLICATION;
    p_app_expr->e.app.f = func;
    p_app_expr->e.app.a = arg;
//    assert(func->type->mode == FUNCTION_TYPE);
    p_app_expr->type = get_type(context, get_expression(manager, func)->type)->type.function.to;
    return app_expr;
}

expr_ptr_t create_constant_expression(expr_manager_t *manager, constant_ptr_t c) {
    expr_ptr_t const_expr = create_expression(manager);
    expr_t *p_const_expr = get_expression(manager, const_expr);
    p_const_expr->mode = CONSTANT;
    p_const_expr->e.constant = c;
    p_const_expr->type = get_constant(manager, c)->type;
    return const_expr;
}

expr_ptr_t create_variable_expression(expr_manager_t *manager, var_ptr_t var) {
    expr_ptr_t var_expr = create_expression(manager);
    expr_t *p_var_expr = get_expression(manager, var_expr);
    p_var_expr->mode = VARIABLE;
    p_var_expr->e.var = var;
    p_var_expr->type = get_variable(manager, var)->type;
    return var_expr;
}

expr_ptr_t create_builtin_expression(expr_manager_t *manager, type_ptr_t type, builtin_function_t builtin) {
    expr_ptr_t builtin_expr = create_expression(manager);
    expr_t *p_builtin_expr = get_expression(manager, builtin_expr);
    p_builtin_expr->mode = BUILTIN;
    p_builtin_expr->e.builtin = builtin;
    p_builtin_expr->type = type;
    return builtin_expr;
}

void set_expression_to_abstraction(context_t *context, expr_manager_t *manager, expr_ptr_t expr, lambda_ptr_t lam) {
    expr_t *e = get_expression(manager, expr);
    lambda_t *p_lam = get_abstraction(manager, lam);
    e->mode = ABSTRACTION;
    e->type = context_create_function_type(context, get_variable(manager, p_lam->var)->type,
                                           get_expression(manager, p_lam->expr)->type);
    e->e.lam = lam;
}

void set_expression_to_application(context_t *context, expr_manager_t *manager, expr_ptr_t expr, expr_ptr_t func, expr_ptr_t arg) {
    expr_t *e = get_expression(manager, expr);
    e->mode = APPLICATION;
    e->type = get_type(context, get_expression(manager, func)->type)->type.function.to;
    e->e.app.f = func;
    e->e.app.a = arg;
}

void set_expression_to_constant(expr_manager_t *manager, expr_ptr_t expr, constant_ptr_t c) {
    expr_t *e = get_expression(manager, expr);
    e->mode = CONSTANT;
    e->type = get_constant(manager, c)->type;
    e->e.constant = c;
}

void set_expression_to_variable(expr_manager_t *manager, expr_ptr_t expr, var_ptr_t var) {
    expr_t *e = get_expression(manager, expr);
    e->mode = VARIABLE;
    e->type = get_variable(manager, var)->type;
    e->e.var = var;
}

void set_expression_to_builtin(expr_manager_t *manager, expr_ptr_t expr, type_ptr_t type, builtin_function_t builtin) {
    expr_t *e = get_expression(manager, expr);
    e->mode = BUILTIN;
    e->type = type;
    e->e.builtin = builtin;
}

expr_t *get_expression(expr_manager_t *manager, expr_ptr_t expr) {
    return &manager->expressions[expr - 1];
}

var_t *get_variable(expr_manager_t *manager, var_ptr_t var) {
    return &manager->variables[var - 1];
}

lambda_t *get_abstraction(expr_manager_t *manager, lambda_ptr_t lam) {
    return &manager->lambdas[lam - 1];
}

constant_t *get_constant(expr_manager_t *manager, constant_ptr_t constant) {
    return &manager->constants[constant - 1];
}

void free_expr_manager(expr_manager_t *manager) {
//    expr_list_t el = manager->expressions;
//
//    while (el) {
//        expr_list_t next = el->tail;
//        free(el);
//        el = next;
//    }

    free(manager->expressions);
    free(manager->variables);
    free(manager->lambdas);
    free(manager->constants);
    free(manager);
}