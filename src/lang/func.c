//
// Created by matthew on 26/10/2020.
//

#include <string.h>
#include "../../include/lang/context.h"
#include "../../include/lang/func.h"

function_builder_t *create_function_builder(void) {
    function_builder_t *builder = (function_builder_t *) malloc(sizeof(function_builder_t));
    builder->lambdas = NULL;
    builder->n_lambdas = 0;
    return builder;
}

lambda_t *builder_create_lambda(function_builder_t *builder) {
    lambda_list_t node = (lambda_list_t) malloc(sizeof(struct lambda_node));
    node->tail = builder->lambdas;
    builder->lambdas = node;
    builder->n_lambdas++;
    node->lambda.var.name = NULL;
    return &node->lambda;
}

func_t *compile_function(context_builder_t *c_builder, function_builder_t *builder, expr_manager_t *expr_manager,
                         const char *function_name, size_t name_length) {
    func_t *func = builder_create_function(c_builder);
    func->name = (char *) malloc(sizeof(char) * (name_length + 1));
    memcpy(func->name, function_name, name_length);
    func->name[name_length] = '\0';

    func->lambdas = (lambda_t *) malloc(sizeof(lambda_t) * builder->n_lambdas);
    func->n_free_variables = builder->n_lambdas;

    // We want to add the lambdas in reverse order because the linked list builds the lambdas backwards

    size_t i = builder->n_lambdas;
    lambda_list_t ll = builder->lambdas;

    while (i--) {
        // Set the binding indices of the original lambdas, so we can match variables up after moving
        // the memory around
        ll->lambda.var.binding_index = i;
        // Set the ith lambda to the head of the list. i counts down, so the list is reversed.
        func->lambdas[i] = ll->lambda;

        // If this isn't the first lambda, we need to update the pointer to the next lambda in the list
        if (i < builder->n_lambdas - 1) {
            func->lambdas[i].expr.e.lam = &func->lambdas[i + 1];
        }
        ll = ll->tail;
    }

    // Update any expression variables in the body (expr of last abstraction) to point to the new locations
    update_expression_vars(&func->lambdas[func->n_free_variables - 1].expr, func->lambdas);

    // Create the functional expression
    func->func_expr = create_function_expression(c_builder, expr_manager, func);
    // Create the entry point expression
    func->entry_point = create_abstraction_expression(c_builder, expr_manager, &func->lambdas[0]);

    return func;
}

void update_expression_vars(expr_t *e, lambda_t *abstractions) {
    switch (e->mode) {
        case VARIABLE:
            // Update the variable pointer to the new location
            e->e.var = &abstractions[e->e.var->binding_index].var;
            break;
        case ABSTRACTION:
            // Recurse on the abstraction body
            update_expression_vars(&e->e.lam->expr, abstractions);
            break;
        case APPLICATION:
            // Recurse on the argument and the function
            update_expression_vars(e->e.app.f, abstractions);
            update_expression_vars(e->e.app.a, abstractions);
            break;
        case FUNCTION:
        case BUILTIN:
        case CONSTANT:
        default:
            // If the expression was a named function, it cannot depend on any variables from this context
            // If the expression is a builtin, it relies only on the stack pointer, so we don't need to update
            // If the expression is a constant, it cannot depend on any variables.
            break;
    }
}

void free_function_builder(function_builder_t *builder) {
    lambda_list_t ll = builder->lambdas;

    while (ll) {
        lambda_list_t next = ll->tail;
        free(ll);
        ll = next;
    }

    free(builder);
}

void free_func(func_t *func) {
    free(func->lambdas);
    free(func->name);
    free(func);
}