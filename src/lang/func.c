//
// Created by matthew on 26/10/2020.
//

#include <string.h>
#include "../../include/lang/expr.h"
#include "../../include/util.h"
#include "../../include/lang/context.h"
#include "../../include/lang/func.h"

function_builder_t *create_function_builder(void) {
    function_builder_t *builder = (function_builder_t *) malloc(sizeof(function_builder_t));
    builder->capacity = 1;
    builder->n_lambdas = 0;
    builder->lambdas = (lambda_ptr_t *) malloc(sizeof(lambda_ptr_t));
    return builder;
}

lambda_ptr_t builder_create_lambda(expr_manager_t *manager, function_builder_t *builder) {
//    lambda_list_t node = (lambda_list_t) malloc(sizeof(struct lambda_node));
//    node->tail = builder->lambdas;
//    builder->lambdas = node;
//    builder->n_lambdas++;
//    node->lambda.var.name = NULL;
//    return &node->lambda;
    if (manager->n_lambdas + 1 > manager->lam_capacity) {
        manager->lam_capacity *= 2;
        manager->lambdas = (lambda_t *) realloc(manager->lambdas, sizeof(lambda_t) * manager->lam_capacity);
    }
    if (builder->n_lambdas + 1 > builder->capacity) {
        builder->capacity *= 2;
        builder->lambdas = (lambda_ptr_t *) realloc(builder->lambdas, sizeof(lambda_ptr_t) * builder->capacity);
    }
    manager->lambdas[manager->n_lambdas].var = create_variable(manager);
    lambda_ptr_t lam = ++manager->n_lambdas;
    builder->lambdas[builder->n_lambdas++] = lam;
    return lam;
}

func_ptr_t compile_function(context_t *context, function_builder_t *builder, expr_manager_t *expr_manager,
                            expr_ptr_t body, const char *function_name, size_t name_length) {
    func_ptr_t func = context_create_function(context);
    func_t *p_func = get_function(context, func);
    p_func->name = (char *) malloc(sizeof(char) * (name_length + 1));
    memcpy(p_func->name, function_name, name_length);
    p_func->name[name_length] = '\0';

//    func->lambdas = (lambda_t *) malloc(sizeof(lambda_t) * builder->n_lambdas);
    p_func->n_free_variables = builder->n_lambdas;

    lambda_ptr_t last_lambda = builder->lambdas[0];
    lambda_t *p_last_lambda = get_abstraction(expr_manager, last_lambda);
    get_variable(expr_manager, p_last_lambda->var)->binding_index = 0;

    for (size_t i = 1; i < builder->n_lambdas; i++) {
        lambda_ptr_t lam = builder->lambdas[i];
        lambda_t *p_lam = get_abstraction(expr_manager, lam);
        get_variable(expr_manager, p_lam->var)->binding_index = i;

        p_last_lambda->expr = create_abstraction_expression(context, expr_manager, lam);
        p_last_lambda = p_lam;
    }

    p_last_lambda->expr = body;

//    // We want to add the lambdas in reverse order because the linked list builds the lambdas backwards
//
//    size_t i = builder->n_lambdas;
//    lambda_list_t ll = builder->lambdas;
//
//    while (i--) {
//        // Set the binding indices of the original lambdas, so we can match variables up after moving
//        // the memory around
//        ll->lambda.var.binding_index = i;
//        // Set the ith lambda to the head of the list. i counts down, so the list is reversed.
//        func->lambdas[i] = ll->lambda;
//
//        // If this isn't the first lambda, we need to update the pointer to the next lambda in the list
//        if (i < builder->n_lambdas - 1) {
//            func->lambdas[i].expr.e.lam = &func->lambdas[i + 1];
//        }
//        ll = ll->tail;
//    }
//
//    // Update any expression variables in the body (expr of last abstraction) to point to the new locations
//    update_expression_vars(&func->lambdas[func->n_free_variables - 1].expr, func->lambdas);

    // Create the functional expression
    p_func->func_expr = create_function_expression(context, expr_manager, func);
    // Create the entry point expression
    p_func->entry_point = create_abstraction_expression(context, expr_manager, builder->lambdas[0]);

    return func;
}

var_ptr_t get_function_variable(expr_manager_t *expr_manager, function_builder_t *builder,
                                const char *name, size_t name_length) {
    for (lambda_ptr_t *p_lam = builder->lambdas; p_lam < builder->lambdas + builder->n_lambdas; p_lam++) {
        var_ptr_t p_var = get_abstraction(expr_manager, *p_lam)->var;
        var_t *var = get_variable(expr_manager, p_var);
        size_t var_name_length = strlen(var->name);
        if (var_name_length == name_length && string_equal(name, var->name, var_name_length)) {
            return p_var;
        }
    }
    return NULL_PTR;
}

//void update_expression_vars(expr_t *e, lambda_t *abstractions) {
//    switch (e->mode) {
//        case VARIABLE:
//            // Update the variable pointer to the new location
//            e->e.var = &abstractions[e->e.var->binding_index].var;
//            break;
//        case ABSTRACTION:
//            // Recurse on the abstraction body
//            update_expression_vars(&e->e.lam->expr, abstractions);
//            break;
//        case APPLICATION:
//            // Recurse on the argument and the function
//            update_expression_vars(e->e.app.f, abstractions);
//            update_expression_vars(e->e.app.a, abstractions);
//            break;
//        case FUNCTION:
//        case BUILTIN:
//        case CONSTANT:
//        default:
//            // If the expression was a named function, it cannot depend on any variables from this context
//            // If the expression is a builtin, it relies only on the stack pointer, so we don't need to update
//            // If the expression is a constant, it cannot depend on any variables.
//            break;
//    }
//}

void free_function_builder(function_builder_t *builder) {
//    lambda_list_t ll = builder->lambdas;
//
//    while (ll) {
//        lambda_list_t next = ll->tail;
//        free(ll);
//        ll = next;
//    }

    free(builder->lambdas);
    free(builder);
}

//void free_func(func_t *func) {
//    free(func->lambdas);
//    free(func->name);
//    free(func);
//}