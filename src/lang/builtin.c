//
// Created by matthew on 23/10/2020.
//

#include <stdio.h>
#include <assert.h>
#include "../../include/interpreter/interpreter.h"
#include "../../include/lang/context.h"
#include "../../include/lang/builtin.h"

void populate_builtins(context_builder_t *builder, expr_manager_t *expr_manager) {
    builder_create_base_type(builder, "poly", 4);
    type_t *number_type = builder_create_base_type_with_print(builder, "Number", 6, &print_number_type);

    populate_numeric_binary_builtin(builder, expr_manager, number_type, &builtin_add, "+", 1);
    populate_numeric_binary_builtin(builder, expr_manager, number_type, &builtin_sub, "-", 1);
    populate_numeric_binary_builtin(builder, expr_manager, number_type, &builtin_mul, "*", 1);
    populate_numeric_binary_builtin(builder, expr_manager, number_type, &builtin_div, "/", 1);
}

void populate_numeric_binary_builtin(context_builder_t *builder, expr_manager_t *expr_manager,
                                     type_t *number_type, builtin_function_t numeric_binary_op,
                                     const char *name, size_t name_length) {
    function_builder_t *f_builder = create_function_builder();

    lambda_t *first = builder_create_lambda(f_builder), *second = builder_create_lambda(f_builder);
    set_variable(&first->var, NULL, 0, number_type);
    set_variable(&second->var, NULL, 0, number_type);
    first->var.binding_index = 0;
    second->var.binding_index = 1;

    set_expression_to_builtin(&second->expr, number_type, numeric_binary_op);
    set_expression_to_abstraction(builder, &first->expr, second);

    compile_function(builder, f_builder, expr_manager, name, name_length);

    free_function_builder(f_builder);
}

void print_number_type(value_t *number_value) {
    printf("%d", *((int *) number_value));
}


expr_t *builtin_create_numeric_binary_op(expr_manager_t *manager, call_stack_t *call_stack,
                                                int *first, int *second, int **result) {
    // Assert that the stack frame has two parameters
    assert(get_stack_frame_size(call_stack) == 2);
    // Get the stack pointer
    call_stack_ptr ptr = get_call_stack_ptr(call_stack);
    // Get the argument expressions
    expr_t *e_first = ptr[0]; // evaluate_expr(manager, ptr[0], call_stack);
    expr_t *e_second = ptr[1]; // evaluate_expr(manager, ptr[1], call_stack);
    // Assert that these are both constant
    assert(e_first->mode == CONSTANT);
    assert(e_second->mode == CONSTANT);
    // Get the integer values of each term
    *first = *((int *) e_first->e.constant->value);
    *second = *((int *) e_second->e.constant->value);
    // Create an integer for the result
    *result = (int *) malloc(sizeof(int));
    // Create a constant and return an expression of this constant
    constant_t *c_result = create_constant((void *) *result, e_first->type);
    return create_constant_expression(manager, c_result);
}

expr_t *builtin_add(expr_manager_t *manager, call_stack_t *call_stack) {
    int first, second;
    int *result;
    expr_t *expr = builtin_create_numeric_binary_op(manager, call_stack, &first, &second, &result);
    *result = first + second;
    return expr;
}

expr_t *builtin_sub(expr_manager_t *manager, call_stack_t *call_stack) {
    int first, second;
    int *result;
    expr_t *expr = builtin_create_numeric_binary_op(manager, call_stack, &first, &second, &result);
    *result = second - first;
    return expr;
}

expr_t *builtin_mul(expr_manager_t *manager, call_stack_t *call_stack) {
    int first, second;
    int *result;
    expr_t *expr = builtin_create_numeric_binary_op(manager, call_stack, &first, &second, &result);
    *result = first * second;
    return expr;
}

expr_t *builtin_div(expr_manager_t *manager, call_stack_t *call_stack) {
    int first, second;
    int *result;
    expr_t *expr = builtin_create_numeric_binary_op(manager, call_stack, &first, &second, &result);
    *result = second / first;
    return expr;
}