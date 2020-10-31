//
// Created by matthew on 26/10/2020.
//

#include <stdio.h>
#include <assert.h>
#include "../../include/lang/context.h"
#include "../../include/lang/expr.h"

expr_manager_t *create_expr_manager(void) {
    expr_manager_t *manager = (expr_manager_t *) malloc(sizeof(expr_manager_t));
    manager->expressions = NULL;
    return manager;
}

expr_t *create_expression(expr_manager_t *manager) {
    expr_list_t node = (expr_list_t) malloc(sizeof(struct expr_node));
    node->tail = manager->expressions;
    manager->expressions = node;
    return &node->expr;
}

expr_t *create_abstraction_expression(context_builder_t *builder, expr_manager_t *manager, lambda_t *lam) {
    expr_t *lam_expr = create_expression(manager);
    lam_expr->mode = ABSTRACTION;
    lam_expr->e.lam = lam;
    lam_expr->type = builder_create_function_type(builder, lam->var.type, lam->expr.type);
    return lam_expr;
}

expr_t *create_function_expression(context_builder_t *builder, expr_manager_t *manager, func_t *func) {
    expr_t *func_expr = create_expression(manager);
    func_expr->mode = FUNCTION;
    func_expr->e.func = func;
    func_expr->type = builder_create_function_type(builder, func->lambdas[0].var.type, func->lambdas[0].expr.type);
    return func_expr;
}

expr_t *create_application_expression(expr_manager_t *manager, expr_t *func, expr_t *arg) {
    expr_t *app_expr = create_expression(manager);
    app_expr->mode = APPLICATION;
    app_expr->e.app.f = func;
    app_expr->e.app.a = arg;
//    assert(func->type->mode == FUNCTION_TYPE);
    app_expr->type = func->type->type.function.to;
    return app_expr;
}

expr_t *create_constant_expression(expr_manager_t *manager, constant_t *c) {
    expr_t *const_expr = create_expression(manager);
    const_expr->mode = CONSTANT;
    const_expr->e.constant = c;
    const_expr->type = c->type;
    return const_expr;
}

expr_t *create_variable_expression(expr_manager_t *manager, var_t *var) {
    expr_t *var_expr = create_expression(manager);
    var_expr->mode = VARIABLE;
    var_expr->e.var = var;
    var_expr->type = var->type;
    return var_expr;
}

void set_expression_to_abstraction(context_builder_t *builder, expr_t *expr, lambda_t *lam) {
    expr->mode = ABSTRACTION;
    expr->type = builder_create_function_type(builder, lam->var.type, lam->expr.type);
    expr->e.lam = lam;
}

void set_expression_to_application(expr_t *expr, expr_t *func, expr_t *arg) {
    expr->mode = APPLICATION;
    expr->type = func->e.lam->expr.type;
    expr->e.app.f = func;
    expr->e.app.a = arg;
}

void set_expression_to_constant(expr_t *expr, constant_t *c) {
    expr->mode = CONSTANT;
    expr->type = c->type;
    expr->e.constant = c;
}

void set_expression_to_variable(expr_t *expr, var_t *var) {
    expr->mode = VARIABLE;
    expr->type = var->type;
    expr->e.var = var;
}

void set_expression_to_builtin(expr_t *expr, type_t *type, builtin_function_t builtin) {
    expr->mode = BUILTIN;
    expr->type = type;
    expr->e.builtin = builtin;
}

void free_expr_manager(expr_manager_t *manager) {
    expr_list_t el = manager->expressions;

    while (el) {
        expr_list_t next = el->tail;
        free(el);
        el = next;
    }

    free(manager);
}