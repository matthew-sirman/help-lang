//
// Created by matthew on 23/10/2020.
//

#include <string.h>
#include <assert.h>
#include "../../include/util.h"
#include "../../include/interpreter/string_buffer.h"
#include "../../include/interpreter/interpreter.h"

#define INPUT_BUFF_SIZE 1024

void start_repl_interpreter(context_t *context, expr_manager_t *expr_manager) {
    char input_buffer[INPUT_BUFF_SIZE];

    printf("*** HELP REPL ***\n");

    int repl_running = 1;

    string_buffer_t *buffer = create_string_buffer(INPUT_BUFF_SIZE);

    while (repl_running) {
        printf(buffer_empty(buffer) ? "> " : "  ");
        fgets(input_buffer, INPUT_BUFF_SIZE, stdin);

        // Get the length - 1 to remove the newline char
        size_t len = strlen(input_buffer) - 1;

        if (input_buffer[0] == ':') {
            switch (handle_command(&input_buffer[1], len - 1)) {
                case CMD_INVALID:
                    printf("Unrecognised command \"%s\"\n", &input_buffer[1]);
                    break;
                case CMD_EXIT:
                    repl_running = 0;
                    break;
            }

            continue;
        }

        for (const char *check = input_buffer; check < input_buffer + INPUT_BUFF_SIZE; check++) {
            if (*check == '\0') {
                append_string(buffer, input_buffer, len);
                break;
            }
            if (*check == ';') {
                append_string(buffer, input_buffer, check - input_buffer + 1);
                execute_statement(context, expr_manager, get_string_start(buffer), get_string_end(buffer));
                reset_buffer(buffer);
                break;
            }
        }
    }

    free_string_buffer(buffer);

    printf("Done.\n");
}

static cmd_code_t handle_command(const char *cmd, size_t len) {
    if (string_equal(cmd, "exit", len)) {
        return CMD_EXIT;
    }

    return CMD_INVALID;
}

void execute_statement(context_t *context, expr_manager_t *expr_manager, const char *stmt_begin, const char *stmt_end) {
    expr_ptr_t expr = parse_expression(context, expr_manager, &stmt_begin, stmt_end);
    expr_ptr_t evaluated = evaluate(context, expr_manager, expr);

    if (evaluated) {
        print_expr(context, expr_manager, evaluated);
    }
}

expr_ptr_t parse_expression(context_t *context, expr_manager_t *expr_manager, const char **e_begin, const char *e_end) {
    // First, skip over any spaces
    while (**e_begin == ' ' && *e_begin < e_end) {
        (*e_begin)++;
    }

    // Extract the first term
    expr_ptr_t expr = parse_expression_term(context, expr_manager, e_begin, e_end);

    // Propagate any potential NULL
    if (!expr) {
        return NULL_PTR;
    }

    // Skip over spaces
    while (**e_begin == ' ' && *e_begin < e_end) {
        (*e_begin)++;
    }

    // If this is the end of a sub expression, return the expression as is
    if (**e_begin == ';' || **e_begin == ')') {
        return expr;
    }

    // Otherwise, this is an application

    // While we still have terms
    while (**e_begin != ';' && **e_begin != ')') {
        // Parse the first applied term
        expr_ptr_t app_arg = parse_expression_term(context, expr_manager, e_begin, e_end);
        // Apply the term to the current application tree
        expr = create_application_expression(context, expr_manager, expr, app_arg);
        // Skip over any spaces
        while (**e_begin == ' ' && *e_begin < e_end) {
            (*e_begin)++;
        }
    }

    return expr;
}

expr_ptr_t
parse_expression_term(context_t *context, expr_manager_t *expr_manager, const char **e_begin, const char *e_end) {
    // If the term starts with a parenthesis, it is a sub expression
    if (**e_begin == '(') {
        (*e_begin)++;
        expr_ptr_t sub_expr = parse_expression(context, expr_manager, e_begin, e_end);
        if (**e_begin != ')') {
            printf("Unmatched parenthesis detected.\n");
            return NULL_PTR;
        }
        (*e_begin)++;
        return sub_expr;
    }

    // TODO: Deal with negative numbers

    // If the term starts with a number, it is a constant (literal)
    if (**e_begin >= '0' && **e_begin <= '9') {
        long long *c_val = (long long *) malloc(sizeof(long long));
        *c_val = 0;

        while (**e_begin >= '0' && **e_begin <= '9') {
            *c_val = (*c_val) * 10 + (**e_begin - '0');
            (*e_begin)++;
        }

        type_ptr_t number_type = get_type_ptr(context, "Number", 6);

        constant_ptr_t numeric_constant = create_constant(expr_manager, (void *) c_val, number_type);
        return create_constant_expression(expr_manager, numeric_constant);
    }

    // Otherwise, the term should be a function name
    const char *name_begin = *e_begin;
    while (**e_begin != ' ' && **e_begin != ')' && **e_begin != ';' && *e_begin < e_end) {
        (*e_begin)++;
    }
    size_t name_length = (*e_begin) - name_begin;

    // Check if this term is a function
    func_ptr_t func = get_function_ptr(context, name_begin, name_length);
    if (func) {
        return get_function(context, func)->func_expr;
    }

//    for (func_t *func = context->functions; func < context->functions + context->n_functions; func++) {
//        // If we find the function, return it's entry point
//        size_t func_name_length = strlen(func->name);
//        if (name_length == func_name_length && string_equal(func->name, name_begin, name_length)) {
//            return func->func_expr;
//        }
//    }

    // If we failed to find a match, then return NULL and print out a linking error.
    char *unresolved = (char *) malloc(sizeof(char) * (name_length + 1));
    memcpy(unresolved, name_begin, name_length);
    unresolved[name_length] = '\0';
    printf("Failed to resolve symbol \"%s\"\n", unresolved);
    free(unresolved);
    return NULL_PTR;
}

expr_ptr_t evaluate(context_t *context, expr_manager_t *manager, expr_ptr_t e) {
    call_stack_t *call_stack = create_call_stack();

    expr_ptr_t evaluated = evaluate_expr(context, manager, e, call_stack, -1);

    if (call_stack->stack_ptr) {
        printf("Incomplete evaluation.\n");
    }

    free_call_stack(call_stack);

    return evaluated;
}

expr_ptr_t evaluate_expr(context_t *context, expr_manager_t *manager, expr_ptr_t e,
                         call_stack_t *call_stack, size_t offset) {
    // Propagate NULL values
    if (!e) {
        return NULL_PTR;
    }
    switch (get_expression(manager, e)->mode) {
        case VARIABLE: {
//            // If the variable is bound, evaluate the bound expression and return that.
//            // Otherwise, this expression cannot be reduced, so just return it.
            assert(offset != -1);
            size_t bind_idx = get_variable(manager, get_expression(manager, e)->e.var)->binding_index;
            stack_node_t bound_expr = call_stack->stack_mem[offset + bind_idx];
            if (bound_expr.expr) {
                return evaluate_expr(context, manager, bound_expr.expr, call_stack, bound_expr.stack_offset);
            } else {
                return e;
            }
        }
        case APPLICATION: {
            // An application is "calling" a lambda function.

            expr_t *app_expr = get_expression(manager, e);

//            // First we evaluate the argument in the current stack frame
//            expr_ptr_t arg = evaluate_expr(context, manager, app_expr->e.app.a, call_stack);
//            call_stack_ptr arg_stack_ptr = get_call_stack_ptr(call_stack);

            // TODO: Get the active frame at this point. If there is no frame yet, set this to -1. Assert
            //   before evaluating expressions with variables that the scope is not -1. If it is, there is an error.
            //   Store the active frame along with the arg expression in the binding. Evaluate each variable in
            //   its active frame.

            // Next we evaluate the function expression - it may not be a lambda yet, but it will have to be
            // before we can apply an argument to it. This will also allocate a new stack frame if needed
            expr_ptr_t function = evaluate_expr(context, manager, app_expr->e.app.f, call_stack, offset);
            // Next, we need to check that it evaluated to a lambda expression
            if (get_expression(manager, function)->mode != ABSTRACTION) {
                printf("Application failure: cannot apply function to a non-abstraction.\n");
                return NULL_PTR;
            }
            // Next, we bind the argument to the appropriate slot on the stack

            size_t idx = get_variable(manager, get_abstraction(manager, get_expression(manager, function)->e.lam)->var)->binding_index; // function->e.lam->var.binding_index;
//            get_call_stack_ptr(call_stack)[idx] = arg;

            call_stack_emplace_expr(call_stack, idx, app_expr->e.app.a, offset);

//            arg_stack_ptr[idx] = app_expr->e.app.a;

            size_t frame = get_stack_frame(call_stack);

            // Then, we evaluate the reduced function expression, with the fact that the variable is now bound
            expr_ptr_t result = evaluate_expr(context, manager, function, call_stack, get_call_stack_offset(call_stack));

            // After this, we can notify the stack allocator that we are done with this variable in the stack frame
            call_stack_notify_pop(call_stack, frame);

            // Finally, we return the result
            return result;
        }
        case ABSTRACTION: {
            lambda_t *lam = get_abstraction(manager, get_expression(manager, e)->e.lam);

            // First we check if the variable is bound or not.
            expr_ptr_t bound_expr = get_call_stack_ptr(call_stack)[get_variable(manager, lam->var)->binding_index].expr;
            if (bound_expr) {
                // If we have an expression bound to the variable, we can evaluate the lambda's expression
                return evaluate_expr(context, manager, lam->expr, call_stack, offset);
            }
            // If the variable is unbound, this abstraction cannot be further reduced
            return e;
        }
        case FUNCTION: {
            func_t *func = get_function(context, get_expression(manager, e)->e.func);

            // Push a stack frame for this function
            call_stack_push_frame(call_stack, func->n_free_variables);

            // Return the entry point expression to the function
            return func->entry_point;
        }
        case BUILTIN:
            // Call the builtin function, providing it with the expression manager and stack pointer
            return get_expression(manager, e)->e.builtin(context, manager, call_stack);
        case CONSTANT:
        default:
            // A constant cannot be further reduced, so return as is.
            return e;
    }
}

void print_type(context_t *context, type_ptr_t type) {
    type_t *p_type = get_type(context, type);
    switch (p_type->mode) {
        case BASE_TYPE:
            // If we have a base type, we can simply print the name of the type
            printf("%s", get_base_type(context, p_type->type.base)->name);
            break;
        case FUNCTION_TYPE: {
            type_t *from_type = get_type(context, p_type->type.function.from);

            // If the type is a function, the print is more complex.
            if (from_type->mode == BASE_TYPE) {
                // If the "from" type of the function type is itself a base type,
                // we just print the type name directly
                printf("%s", get_base_type(context, from_type->type.base)->name);
            } else {
                // Otherwise, we recurse on the "from" type, encapsulating the expression
                // with parentheses
                printf("(");
                print_type(context, p_type->type.function.from);
                printf(")");
            }
            // We then print an arrow
            printf(" -> ");
            // Finally, we print the "to" type.
            print_type(context, p_type->type.function.to);
            break;
        }
    }
}

void print_value(context_t *context, expr_manager_t *manager, constant_ptr_t val) {
    constant_t *constant = get_constant(manager, val);
    type_t *val_type = get_type(context, constant->type);
    switch (val_type->mode) {
        case BASE_TYPE: {
            base_type_t *base = get_base_type(context, val_type->type.base);
            // If we know how to print a value of this type:
            if (base->print_method != UNKNOWN_PRINT) {
                // Invoke the print method with the value.
                base->print_method(constant->value);
            } else {
                // Otherwise just print <UNKNOWN>
                printf("<UNKNOWN>");
            }
            break;
        }
        case FUNCTION_TYPE:
            // A constant should never have a function type
            return;
    }
}

void print_expr_component(context_t *context, expr_manager_t *manager, expr_ptr_t e) {
    expr_t *expr = get_expression(manager, e);
    switch (expr->mode) {
        case VARIABLE:
            // If we have a variable type, we simply print its name.
            printf("var %s ", get_variable(manager, expr->e.var)->name);
            break;
        case ABSTRACTION:
            // If we have an abstraction (lambda) type, we simply print
            // the word "lambda" to indicate that this is an anonymous function
            printf("lambda");
            break;
        case APPLICATION:
            // An application expression should be printed in the form "(e1) e2".
            printf("(");
            print_expr_component(context, manager, expr->e.app.f);
            printf(") ");
            expr_mode_t mode = get_expression(manager, expr->e.app.a)->mode;
            if (mode != CONSTANT && mode != VARIABLE) {
                printf("(");
                print_expr_component(context, manager, expr->e.app.a);
                printf(")");
            } else {
                print_expr_component(context, manager, expr->e.app.a);
            }
            break;
        case FUNCTION:
            // If this is a function, just pass the call through to the entry point
            print_expr_component(context, manager, get_function(context, expr->e.func)->entry_point);
            break;
        case BUILTIN:
            printf("<builtin>");
            break;
        case CONSTANT:
            // If the expression has a constant type, we can print this following
            // "val".
            printf("val = ");
            // Print the actual value
            print_value(context, manager, expr->e.constant);
            break;
    }
}

void print_expr(context_t *context, expr_manager_t *manager, expr_ptr_t e) {
    // Print the expression recursively
    print_expr_component(context, manager, e);
    // Print a colon to specify that we are now printing the type of the expression
    printf(": ");
    // Print the type itself
    print_type(context, get_expression(manager, e)->type);
    // Print a newline
    printf("\n");
}
