//
// Created by matthew on 23/10/2020.
//

#include <stdio.h>
#include <assert.h>
#include "../../include/interpreter/interpreter.h"
#include "../../include/lang/context.h"
#include "../../include/lang/builtin.h"

void populate_builtins(context_t *context, expr_manager_t *expr_manager) {
    type_ptr_t poly_type = context_create_base_type(context, "poly", 4);
    type_ptr_t number_type = context_create_base_type_with_print(context, "Number", 6, &print_number_type);
    type_ptr_t bool_type = context_create_base_type_with_print(context, "Boolean", 7, &print_bool_type);

    populate_numeric_binary_builtin(context, expr_manager, number_type, &builtin_add, "+", 1);
    populate_numeric_binary_builtin(context, expr_manager, number_type, &builtin_sub, "-", 1);
    populate_numeric_binary_builtin(context, expr_manager, number_type, &builtin_mul, "*", 1);
    populate_numeric_binary_builtin(context, expr_manager, number_type, &builtin_div, "/", 1);
    populate_numeric_binary_builtin(context, expr_manager, number_type, &builtin_mod, "%", 1);

    populate_numeric_comparison_builtin(context, expr_manager, number_type, bool_type, &builtin_gt, ">", 1);

    populate_ifelse_builtin(context, expr_manager, poly_type, bool_type);
}

void populate_numeric_binary_builtin(context_t *context, expr_manager_t *expr_manager,
                                     type_ptr_t number_type, builtin_function_t numeric_binary_op,
                                     const char *name, size_t name_length) {
    function_builder_t *f_builder = create_function_builder(expr_manager);

    lambda_ptr_t first = builder_create_lambda(expr_manager, f_builder), second = builder_create_lambda(expr_manager,
                                                                                                        f_builder);
    lambda_t *p_first = get_abstraction(expr_manager, first), *p_second = get_abstraction(expr_manager, second);
    set_variable(expr_manager, p_first->var, NULL, 0, number_type);
    set_variable(expr_manager, p_second->var, NULL, 0, number_type);
//    get_variable(expr_manager, p_first->var)->binding_index = 0;
//    get_variable(expr_manager, p_second->var)->binding_index = 1;
//
//    set_expression_to_builtin(expr_manager, p_second->expr, number_type, numeric_binary_op);
//    set_expression_to_abstraction(builder, expr_manager, p_first->expr, second);

    expr_ptr_t body = create_builtin_expression(expr_manager, number_type, numeric_binary_op);

    compile_function(context, f_builder, expr_manager, body, name, name_length);

    free_function_builder(f_builder);
}

void populate_numeric_comparison_builtin(context_t *context, expr_manager_t *expr_manager,
                                         type_ptr_t number_type, type_ptr_t bool_type,
                                         builtin_function_t numeric_comparison_op,
                                         const char *name, size_t name_length) {
    function_builder_t *f_builder = create_function_builder(expr_manager);

    lambda_ptr_t first = builder_create_lambda(expr_manager, f_builder), second = builder_create_lambda(expr_manager,
                                                                                                        f_builder);
    lambda_t *p_first = get_abstraction(expr_manager, first), *p_second = get_abstraction(expr_manager, second);
    set_variable(expr_manager, p_first->var, NULL, 0, number_type);
    set_variable(expr_manager, p_second->var, NULL, 0, number_type);

    expr_ptr_t body = create_builtin_expression(expr_manager, bool_type, numeric_comparison_op);

    compile_function(context, f_builder, expr_manager, body, name, name_length);

    free_function_builder(f_builder);
}

void populate_ifelse_builtin(context_t *context, expr_manager_t *expr_manager,
                             type_ptr_t poly_type, type_ptr_t bool_type) {
    function_builder_t *f_builder = create_function_builder(expr_manager);

    lambda_ptr_t condition = builder_create_lambda(expr_manager, f_builder);
    lambda_ptr_t then_clause = builder_create_lambda(expr_manager, f_builder);
    lambda_ptr_t else_clause = builder_create_lambda(expr_manager, f_builder);
    lambda_t *p_cond = get_abstraction(expr_manager, condition);
    lambda_t *p_then = get_abstraction(expr_manager, then_clause);
    lambda_t *p_else = get_abstraction(expr_manager, else_clause);

    type_ptr_t poly_to_bool_type = context_create_function_type(context, poly_type, bool_type);
    set_variable(expr_manager, p_cond->var, NULL, 0, poly_to_bool_type);
    set_variable(expr_manager, p_then->var, NULL, 0, poly_type);
    set_variable(expr_manager, p_else->var, NULL, 0, poly_type);

    expr_ptr_t body = create_builtin_expression(expr_manager, poly_type, &builtin_ifelse);

    compile_function(context, f_builder, expr_manager, body, "ifelse", 6);

    free_function_builder(f_builder);
}

void print_number_type(value_t number_value) {
    printf("%d", *((int *) number_value));
}

void print_bool_type(value_t bool_value) {
    printf((*(bool *) bool_value) ? "true" : "false");
}

expr_ptr_t builtin_numeric_binary_op(context_t *context, expr_manager_t *manager, call_stack_t *call_stack,
                                     int *first, int *second, int **result) {
    // Assert that the stack frame has two parameters
    assert(get_stack_frame_size(call_stack) == 2);
    // Get the stack pointer
    call_stack_ptr ptr = get_call_stack_ptr(call_stack);
    // Get the stack nodes
    stack_node_t first_node = ptr[0], second_node = ptr[1];
    // Get the argument expressions
    expr_ptr_t e_first = evaluate_expr(context, manager, first_node.expr, call_stack, first_node.stack_offset);
    // evaluate_expr(manager, ptr[0], call_stack);
    expr_ptr_t e_second = evaluate_expr(context, manager, second_node.expr, call_stack, second_node.stack_offset);
    // evaluate_expr(manager, ptr[1], call_stack);
    constant_t *c_first = get_constant(manager, get_expression(manager, e_first)->e.constant);
    constant_t *c_second = get_constant(manager, get_expression(manager, e_second)->e.constant);
    // Assert that these are both constant
    assert(get_expression(manager, e_first)->mode == CONSTANT);
    assert(get_expression(manager, e_second)->mode == CONSTANT);
    // Get the integer values of each term
    *first = *((int *) c_first->value);
    *second = *((int *) c_second->value);
    // Create an integer for the result
    *result = (int *) malloc(sizeof(int));
    // Create a constant and return an expression of this constant
    constant_ptr_t c_result = create_constant(manager, (void *) *result, c_first->type);
    return create_constant_expression(manager, c_result);
}

expr_ptr_t builtin_numeric_comparison_op(context_t *context, expr_manager_t *manager, call_stack_t *call_stack,
                                         int *first, int *second, bool **result) {
    // Assert that the stack frame has two parameters
    assert(get_stack_frame_size(call_stack) == 2);
    // Get the stack pointer
    call_stack_ptr ptr = get_call_stack_ptr(call_stack);
    // Get the stack nodes
    stack_node_t first_node = ptr[0], second_node = ptr[1];
    // Get the argument expressions
    expr_ptr_t e_first = evaluate_expr(context, manager, first_node.expr, call_stack, first_node.stack_offset);
    // evaluate_expr(manager, ptr[0], call_stack);
    expr_ptr_t e_second = evaluate_expr(context, manager, second_node.expr, call_stack, second_node.stack_offset);
    // evaluate_expr(manager, ptr[1], call_stack);
    constant_t *c_first = get_constant(manager, get_expression(manager, e_first)->e.constant);
    constant_t *c_second = get_constant(manager, get_expression(manager, e_second)->e.constant);
    // Assert that these are both constant
    assert(get_expression(manager, e_first)->mode == CONSTANT);
    assert(get_expression(manager, e_second)->mode == CONSTANT);
    // Get the integer values of each term
    *first = *((int *) c_first->value);
    *second = *((int *) c_second->value);
    // Create an integer for the result
    *result = (bool *) malloc(sizeof(bool));
    // Create a constant and return an expression of this constant
    // TODO: somehow source the bool type dynamically
    constant_ptr_t c_result = create_constant(manager, (void *) *result, 3);
    return create_constant_expression(manager, c_result);
}

expr_ptr_t builtin_add(context_t *context, expr_manager_t *manager, call_stack_t *call_stack) {
    int first, second;
    int *result;
    expr_ptr_t expr = builtin_numeric_binary_op(context, manager, call_stack, &first, &second, &result);
    *result = first + second;
    return expr;
}

expr_ptr_t builtin_sub(context_t *context, expr_manager_t *manager, call_stack_t *call_stack) {
    int first, second;
    int *result;
    expr_ptr_t expr = builtin_numeric_binary_op(context, manager, call_stack, &first, &second, &result);
    *result = second - first;
    return expr;
}

expr_ptr_t builtin_mul(context_t *context, expr_manager_t *manager, call_stack_t *call_stack) {
    int first, second;
    int *result;
    expr_ptr_t expr = builtin_numeric_binary_op(context, manager, call_stack, &first, &second, &result);
    *result = first * second;
    return expr;
}

expr_ptr_t builtin_div(context_t *context, expr_manager_t *manager, call_stack_t *call_stack) {
    int first, second;
    int *result;
    expr_ptr_t expr = builtin_numeric_binary_op(context, manager, call_stack, &first, &second, &result);
    *result = second / first;
    return expr;
}

expr_ptr_t builtin_mod(context_t *context, expr_manager_t *manager, call_stack_t *call_stack) {
    int first, second;
    int *result;
    expr_ptr_t expr = builtin_numeric_binary_op(context, manager, call_stack, &first, &second, &result);
    *result = second % first;
    return expr;
}

expr_ptr_t builtin_gt(context_t *context, expr_manager_t *manager, call_stack_t *call_stack) {
    int first, second;
    bool *result;
    expr_ptr_t expr = builtin_numeric_comparison_op(context, manager, call_stack, &first, &second, &result);
    *result = second > first;
    return expr;
}

expr_ptr_t builtin_ifelse(context_t *context, expr_manager_t *manager, call_stack_t *call_stack) {
    // Assert that the stack frame has two parameters
    assert(get_stack_frame_size(call_stack) == 3);
    // Get the stack pointer
    call_stack_ptr ptr = get_call_stack_ptr(call_stack);
    stack_node_t cond = ptr[0], then_branch = ptr[1], else_branch = ptr[2];
    // Get the integer values of each term
    bool condition = *((bool *) get_constant(
            manager,
            get_expression(manager, evaluate_expr(context, manager, cond.expr, call_stack, cond.stack_offset))->e.constant
    )->value);
    return condition ? evaluate_expr(context, manager, then_branch.expr, call_stack, then_branch.stack_offset) :
           evaluate_expr(context, manager, else_branch.expr, call_stack, else_branch.stack_offset);
}