//
// Created by matthew on 23/10/2020.
//

#include <string.h>
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
    expr_t *expr = parse_expression(context, expr_manager, &stmt_begin, stmt_end);
    expr_t *evaluated = evaluate(expr_manager, expr);

    if (evaluated) {
        print_expr(evaluated);
    }
}

expr_t *parse_expression(context_t *context, expr_manager_t *expr_manager, const char **e_begin, const char *e_end) {
    // First, skip over any spaces
    while (**e_begin == ' ' && *e_begin < e_end) {
        (*e_begin)++;
    }

    // Extract the first term
    expr_t *expr = parse_expression_term(context, expr_manager, e_begin, e_end);

    // Propagate any potential NULL
    if (!expr) {
        return NULL;
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
        expr_t *app_arg = parse_expression_term(context, expr_manager, e_begin, e_end);
        // Apply the term to the current application tree
        expr = create_application_expression(expr_manager, expr, app_arg);
        // Skip over any spaces
        while (**e_begin == ' ' && *e_begin < e_end) {
            (*e_begin)++;
        }
    }

    return expr;
}

expr_t *parse_expression_term(context_t *context, expr_manager_t *expr_manager, const char **e_begin, const char *e_end) {
    // If the term starts with a parenthesis, it is a sub expression
    if (**e_begin == '(') {
        (*e_begin)++;
        expr_t *sub_expr = parse_expression(context, expr_manager, e_begin, e_end);
        if (**e_begin != ')') {
            fprintf(stderr, "Unmatched parenthesis detected.\n");
            return NULL;
        }
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

        type_t *number_type = get_type(context, "Number", 6);

        constant_t *numeric_constant = create_constant((void *) c_val, number_type);
        return create_constant_expression(expr_manager, numeric_constant);
    }

    // Otherwise, the term should be a function name
    const char *name_begin = *e_begin;
    while (**e_begin != ' ' && **e_begin != ')' && **e_begin != ';' && *e_begin < e_end) {
        (*e_begin)++;
    }
    size_t name_length = (*e_begin) - name_begin;

    // Check if this term is a function
    for (func_t *func = context->functions; func < context->functions + context->n_functions; func++) {
        // If we find the function, return it's entry point
        size_t func_name_length = strlen(func->name);
        if (name_length == func_name_length && string_equal(func->name, name_begin, name_length)) {
            return func->func_expr;
        }
    }

    // If we failed to find a match, then return NULL and print out a linking error.
    char *unresolved = (char *) malloc(sizeof(char) * (name_length + 1));
    memcpy(unresolved, name_begin, name_length);
    unresolved[name_length] = '\0';
    fprintf(stderr, "Failed to resolve symbol \"%s\"\n", unresolved);
    free(unresolved);
    return NULL;
}

expr_t *evaluate(expr_manager_t *manager, expr_t *e) {
    call_stack_t *call_stack = create_call_stack();

    expr_t *evaluated = evaluate_expr(manager, e, call_stack);

    if (call_stack->stack_ptr) {
        printf("Incomplete evaluation.\n");
    }

    free_call_stack(call_stack);

    return evaluated;
}

expr_t *evaluate_expr(expr_manager_t *manager, expr_t *e, call_stack_t *call_stack) {
    // Propagate NULL values
    if (!e) {
        return NULL;
    }
    switch (e->mode) {
        case VARIABLE: {
            // If the variable is bound, evaluate the bound expression and return that.
            // Otherwise, this expression cannot be reduced, so just return it.
            expr_t *bound_expr = get_call_stack_ptr(call_stack)[e->e.var->binding_index];
            if (bound_expr) {
                return evaluate_expr(manager, bound_expr, call_stack);
            } else {
                return e;
            }
        }
        case APPLICATION: {
            // An application is "calling" a lambda function.


            // First we evaluate the argument in the current stack frame
            expr_t *arg = evaluate_expr(manager, e->e.app.a, call_stack);

            // Next we evaluate the function expression - it may not be a lambda yet, but it will have to be
            // before we can apply an argument to it. This will also allocate a new stack frame if needed
            expr_t *function = evaluate_expr(manager, e->e.app.f, call_stack);
            // Next, we need to check that it evaluated to a lambda expression
            if (function->mode != ABSTRACTION) {
                fprintf(stderr, "Application failure: cannot apply function to a non-abstraction.\n");
                return NULL;
            }
            // Next, we bind the argument to the appropriate slot on the stack

            size_t idx = function->e.lam->var.binding_index;
            get_call_stack_ptr(call_stack)[idx] = arg;

            // Then, we evaluate the reduced function expression, with the fact that the variable is now bound
            expr_t *result = evaluate_expr(manager, function, call_stack);

            // After this, we can notify the stack allocator that we are done with this variable in the stack frame
            call_stack_notify_pop(call_stack);

            // Finally, we return the result
            return result;
        }
        case ABSTRACTION: {
            // First we check if the variable is bound or not.
            expr_t *bound_expr = get_call_stack_ptr(call_stack)[e->e.lam->var.binding_index];
            if (bound_expr) {
                // If we have an expression bound to the variable, we can evaluate the lambda's expression
                return evaluate_expr(manager, &e->e.lam->expr, call_stack);
            }
            // If the variable is unbound, this abstraction cannot be further reduced
            return e;
        }
        case FUNCTION: {
            // Push a stack frame for this function
            call_stack_push_frame(call_stack, e->e.func->n_free_variables);

            // Return the entry point expression to the function
            return e->e.func->entry_point;
        }
        case BUILTIN:
            // Call the builtin function, providing it with the expression manager and stack pointer
            return e->e.builtin(manager, call_stack);
        case CONSTANT:
        default:
            // A constant cannot be further reduced, so return as is.
            return e;
    }
}

void print_type(type_t *type) {
    switch (type->mode) {
        case BASE_TYPE:
            // If we have a base type, we can simply print the name of the type
            printf("%s", type->type.base->name);
            break;
        case FUNCTION_TYPE:
            // If the type is a function, the print is more complex.
            if (type->type.function.from->mode == BASE_TYPE) {
                // If the "from" type of the function type is itself a base type,
                // we just print the type name directly
                printf("%s", type->type.function.from->type.base->name);
            } else {
                // Otherwise, we recurse on the "from" type, encapsulating the expression
                // with parentheses
                printf("(");
                print_type(type->type.function.from);
                printf(")");
            }
            // We then print an arrow
            printf(" -> ");
            // Finally, we print the "to" type the same as the "from" type.
            if (type->type.function.to->mode == BASE_TYPE) {
                printf("%s", type->type.function.to->type.base->name);
            } else {
                printf("(");
                print_type(type->type.function.to);
                printf(")");
            }
            break;
    }
}

void print_value(constant_t *val) {
    switch (val->type->mode) {
        case BASE_TYPE:
            // If we know how to print a value of this type:
            if (val->type->type.base->print_method != UNKNOWN_PRINT) {
                // Invoke the print method with the value.
                (*val->type->type.base->print_method)(val->value);
            } else {
                // Otherwise just print <UNKNOWN>
                printf("<UNKNOWN>");
            }
            break;
        case FUNCTION_TYPE:
            // A constant should never have a function type
            return;
    }
}

void print_expr_component(expr_t *e) {
    switch (e->mode) {
        case VARIABLE:
            // If we have a variable type, we simply print its name.
            printf("var %s ", e->e.var->name);
            break;
        case ABSTRACTION:
            // If we have an abstraction (lambda) type, we simply print
            // the word "lambda" to indicate that this is an anonymous function
            printf("lambda");
            break;
        case APPLICATION:
            // An application expression should be printed in the form "(e1) e2".
            printf("(");
            print_expr_component(e->e.app.f);
            printf(") ");
            if (e->e.app.a->mode != CONSTANT && e->e.app.a->mode != VARIABLE) {
                printf("(");
                print_expr_component(e->e.app.a);
                printf(")");
            } else {
                print_expr_component(e->e.app.a);
            }
            break;
        case FUNCTION:
            // If this is a function, just pass the call through to the entry point
            print_expr_component(e->e.func->entry_point);
            break;
        case BUILTIN:
            printf("<builtin>");
            break;
        case CONSTANT:
            // If the expression has a constant type, we can print this following
            // "val".
            printf("val = ");
            // Print the actual value
            print_value(e->e.constant);
            break;
    }
}

void print_expr(expr_t *e) {
    // Print the expression recursively
    print_expr_component(e);
    // Print a colon to specify that we are now printing the type of the expression
    printf(": ");
    // Print the type itself
    print_type(e->type);
    // Print a newline
    printf("\n");
}
