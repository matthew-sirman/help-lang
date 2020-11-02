//
// Created by matthew on 23/10/2020.
//

#include "../../include/util.h"
#include "../../include/lang/expr.h"
#include "../../include/compiler/parser.h"

/* func constant: -> 5;
 * func inc: x -> (+ x 1);
 * func add_const: x -> (+ x constant);
 * type List<T> ::= nil {} | cons {T * List<T>};
 * // constant has type: Number
 * // inc has type: Number -> Number
 * // add_const has type: Number -> Number
 *
 * */

parse_error_t parse_source(context_t *context, expr_manager_t *expr_manager, const char *source, size_t length) {
    // All declarations are either a "type" or a "func". We also include a "NONE" token (e.g. for after the last
    // token is found, or if the file is empty) and an "ERROR" token (e.g. for an incomplete declaration)

    const char *dec_begin, *dec_end;
    const char *read_point = source, *source_end = source + length;

    unsigned char scanning = 1;

    while (scanning) {
        switch (extract_declaration(read_point, source_end, &dec_begin, &dec_end)) {
            case NONE:
                scanning = 0;
                break;
            case FUNC:
                if (parse_function(context, expr_manager, dec_begin, dec_end)) {
                    return MALFORMED_FUNC;
                }
                break;
            case TYPE:
                if (parse_type(context, dec_begin, dec_end)) {
                    return MALFORMED_TYPE;
                }
                break;
            case ERROR:
                return INCOMPLETE_DECLARATION;
        }

        read_point = dec_end;
    }

    return SUCCESS;
}

token_t extract_declaration(const char *s_begin, const char *s_end, const char **r_begin, const char **r_end) {
    token_t t = NONE;
    while (s_begin < s_end) {
        if (string_equal(s_begin, "func", 4)) {
            t = FUNC;
            *r_begin = s_begin;
            break;
        } else if (string_equal(s_begin, "type", 4)) {
            t = TYPE;
            *r_begin = s_begin;
            break;
        }
        s_begin++;
    }
    if (!t) {
        return t;
    }

    while (s_begin < s_end) {
        if (*s_begin == ';') {
            *r_end = s_begin;
            return t;
        }
        s_begin++;
    }

    return ERROR;
}

int parse_function(context_t *context, expr_manager_t *expr_manager, const char *f_begin, const char *f_end) {
    /* FORMAT:
     * func name: [arg1 arg2 ...] | _ -> expr;
     * */

    // Jump 4 chars forward to skip over the "func" keyword
    f_begin += 4;

    // Check the next character is a space
    if (*f_begin != ' ') {
        fprintf(stderr, "\"func\" keyword must be followed by a space before name specifier.\n");
        return 1;
    }

    // Skip over spaces until we reach a non-space
    while (*f_begin == ' ' && f_begin < f_end) {
        f_begin++;
    }

    // Extract the function name from the string
    const char *name_begin = f_begin, *name_end = NULL;
    while (f_begin < f_end) {
        if (*f_begin == ':' || *f_begin == ' ') {
            name_end = f_begin;
            break;
        }
        f_begin++;
    }

    // Skip over spaces
    while (*f_begin == ' ' && f_begin < f_end) {
        f_begin++;
    }

    // Check that (1) the function name ended with a colon and (2) the function name was
    // non-empty
    if (!name_end || *f_begin != ':') {
        fprintf(stderr, "Function name must be followed by \":\".\n");
        return 1;
    }
    if (name_begin == name_end) {
        fprintf(stderr, "Function name cannot be empty.\n");
        return 1;
    }

    size_t name_length = name_end - name_begin;

    // We know that the next character is a colon, so we can just skip over it without checking
    f_begin++;

    // Create a function builder
    function_builder_t *f_builder = create_function_builder(expr_manager);
    builder_set_name(f_builder, name_begin, name_length);

    // Get a reference to the polymorphic type
    type_ptr_t poly_type = get_type_ptr(context, "poly", 4);

    // If the poly type does not yet exist, we must create it.
    if (!poly_type) {
        poly_type = context_create_base_type(context, "poly", 4);
    }

    // First, we parse the arg list
    while (1) {
        // Skip over spaces
        while (*f_begin == ' ' && f_begin < f_end) {
            f_begin++;
        }
        // Check if the "arrow" operator is found - this indicates that the arg list is finished
        if (string_equal(f_begin, "->", 2)) {
            break;
        } else if (*f_begin == '-') {
            // If the arg list has a '-' character but not the arrow operator, it is a malformed function
            fprintf(stderr, "Invalid symbol \"-\" used in function argument list.\n");
            // Cleanup and return
            free_function_builder(f_builder);
            return 1;
        }

        // Otherwise, scan over the next argument and add to the list
        const char *arg_name_begin = f_begin;
        while (*f_begin != ' ' && *f_begin != '-' && f_begin < f_end) {
            f_begin++;
        }

        // If we reached the end of the string without finding the arrow, this is a malformed function.
        // Otherwise, we found an argument
        if (f_begin == f_end) {
            fprintf(stderr, "No function body detected.\n");
            // Cleanup and return
            free_function_builder(f_builder);
            return 1;
        } else {
            // Create the lambda function for this abstraction
            lambda_ptr_t lam = builder_create_lambda(expr_manager, f_builder);
            // Set the variable name in the lambda
            size_t arg_name_length = f_begin - arg_name_begin;
            // Set the variable to the given name
            set_variable(expr_manager, get_abstraction(expr_manager, lam)->var, arg_name_begin, arg_name_length, poly_type);
        }
    }

    // By this point, we know we have extracted the arguments and reached the "->" symbol, so we can finally
    // parse the function body.
    // First, we can just skip over the arrow and any spaces
    f_begin += 2;
    while (*f_begin == ' ' && f_begin < f_end) {
        f_begin++;
    }

    // Next we parse the expression
    expr_ptr_t body_expr = parse_function_expression(context, f_builder, expr_manager, &f_begin, f_end);

    // If the expression failed to compile, return failure
    if (!body_expr) {
        // Cleanup and return
        free_function_builder(f_builder);
        return 1;
    }

//    lambda_list_t lambda_node = f_builder->lambdas;
//
//    // TODO: This can probably be done without a copy
//    lambda_node->lambda.expr = *body_expr;
//
//    while (lambda_node) {
//        if (lambda_node->tail) {
//            set_expression_to_abstraction(builder, &lambda_node->tail->lambda.expr, &lambda_node->lambda);
//        }
//        lambda_node = lambda_node->tail;
//    }

    compile_function(context, f_builder, expr_manager, body_expr, name_begin, name_length);

    // Cleanup and return success
    free_function_builder(f_builder);
    return 0;
}

expr_ptr_t parse_function_expression(context_t *context, function_builder_t *f_builder,
                                     expr_manager_t *expr_manager, const char **e_begin, const char *e_end) {
    // First, skip over any spaces
    while (**e_begin == ' ' && *e_begin < e_end) {
        (*e_begin)++;
    }

    // Extract the first term
    expr_ptr_t expr = parse_function_term(context, f_builder, expr_manager, e_begin, e_end);

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
        expr_ptr_t app_arg = parse_function_term(context, f_builder, expr_manager, e_begin, e_end);
        // Apply the term to the current application tree
        expr = create_application_expression(context, expr_manager, expr, app_arg);
        // Skip over any spaces
        while (**e_begin == ' ' && *e_begin < e_end) {
            (*e_begin)++;
        }
    }

    return expr;
}

expr_ptr_t parse_function_term(context_t *context, function_builder_t  *f_builder,
                               expr_manager_t *expr_manager, const char **e_begin, const char *e_end) {
    // If the term starts with a parenthesis, it is a sub expression
    if (**e_begin == '(') {
        (*e_begin)++;
        expr_ptr_t sub_expr = parse_function_expression(context, f_builder, expr_manager, e_begin, e_end);
        if (**e_begin != ')') {
            fprintf(stderr, "Unmatched parenthesis detected.\n");
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

        if (!number_type) {
            number_type = context_create_base_type_with_print(context, "Number", 6, &print_number_type);
        }

        constant_ptr_t numeric_constant = create_constant(expr_manager, (void *) c_val, number_type);
        return create_constant_expression(expr_manager, numeric_constant);
    }

    // Otherwise, the term should be a function or variable name
    const char *name_begin = *e_begin;
    while (**e_begin != ' ' && **e_begin != ')' && *e_begin < e_end) {
        (*e_begin)++;
    }
    size_t name_length = (*e_begin) - name_begin;

    // First we want to check if it is a variable in the function arguments - variables shadow functions
    // from the outer scope
    var_ptr_t var = get_function_variable(expr_manager, f_builder, name_begin, name_length);
    if (var) {
        return create_variable_expression(expr_manager, var);
    }

    // Next we can check if the term was actually the name of the function itself
    if (name_length == f_builder->name_length && string_equal(name_begin, f_builder->name, name_length)) {
        return f_builder->placeholder_func;
    }

    // If this fell through, we next want to check the function list in the context builder
    func_ptr_t func = context_find_function(context, name_begin, name_length);
    if (func) {
        return get_function(context, func)->func_expr;
    }

//    func_list_t fl = builder->functions;
//
//    // TODO: Handle recursion - func_expr won't yet exist
//
//    while (fl) {
//        size_t func_name_length = strlen(fl->function.name);
//        // If the name matches, create an abstraction expression with the first lambda in the function
//        if (name_length == func_name_length && string_equal(fl->function.name, name_begin, name_length)) {
//            return fl->function.func_expr;
//        }
//        fl = fl->tail;
//    }

    // If we failed to find a match, then return NULL and print out a linking error.
    char *unresolved = (char *) malloc(sizeof(char) * (name_length + 1));
    memcpy(unresolved, name_begin, name_length);
    unresolved[name_length] = '\0';
    fprintf(stderr, "Failed to resolve symbol \"%s\"\n", unresolved);
    free(unresolved);
    return NULL_PTR;
}

int parse_type(context_t *context, const char *t_begin, const char *t_end) {
    return 1;

//    /* FORMAT:
//     * type name ::= opt0 {binding_tuple} | opt1;
//     * */
//
//    // Jump 4 chars forward to skip over the "type" keyword
//    t_begin += 4;
//
//    // Check the next character is a space
//    if (*t_begin != ' ') {
//        return 1;
//    }
//
//    // Skip over spaces until we reach a non-space
//    while (*t_begin == ' ' && t_begin < t_end) {
//        t_begin++;
//    }
//
//    // Extract the type name from the string
//    const char *name_begin = t_begin, *name_end = NULL;
//    while (t_begin < t_end) {
//        if (*t_begin == ':' || *t_begin == ' ') {
//            name_end = t_begin;
//            break;
//        }
//        t_begin++;
//    }
//
//    // Check that (1) the type name ended (i.e. there was a ':' in the string) and (2) the type name was
//    // non-empty
//    if (!name_end || name_begin == name_end) {
//        return 1;
//    }
//
//    // Skip over any spaces
//    while (*t_begin == ' ' && t_begin < t_end) {
//        t_begin++;
//    }
//
//    // If the next encountered symbol isn't "::=", the string is invalid
//    if (!string_equal(t_begin, "::=", 3)) {
//        return 1;
//    }
//
//    t_begin += 3;
//
//    type_comp_list_t components = NULL;
//    size_t n_components = 0;
//
//    // Next we scan through the type declaration for each option component
//    while (1) {
//        // Skip over any spaces
//        while ((*t_begin == ' ' || *t_begin == '|') && t_begin < t_end) {
//            t_begin++;
//        }
//
//        // Scan for the option name
//        // TODO: Check for invalid characters (e.g. -)
//        const char *c_name_begin = t_begin;
//        while (*t_begin != ' ' && *t_begin != '{' && *t_begin != '|' && t_begin < t_end) {
//            t_begin++;
//        }

//        type_comp_list_t component = (type_comp_list_t) malloc(sizeof(struct type_comp_node));
//        size_t c_name_length = t_begin - c_name_begin;
//        component->component.name = (char *) malloc(sizeof(char) * (c_name_length + 1));
//        memcpy(component->component.name, c_name_begin, c_name_length);
//        component->component.name[c_name_length] = '\0';
//
//        component->tail = components;
//        components = component;
//
//        n_components++;
//
//        while (*t_begin != '|' && t_begin < t_end) {
//            t_begin++;
//        }
//
//        if (t_begin == t_end) {
//            break;
//        }
//    }
//
//    // Finally, create the type object in the linked list of types
//    type_list_t type_node = (type_list_t) malloc(sizeof(struct type_node));
//
//    // Set the name of the function to the extracted name
//    size_t name_length = name_end - name_begin;
//    type_node->type.name = (char *) malloc(sizeof(char) * (name_length + 1));
//    memcpy(type_node->type.name, name_begin, name_length);
//    type_node->type.name[name_length] = '\0';

//    type_node->type.components = (struct type_component *) malloc(sizeof(struct type_component) * n_components);
//    type_node->type.n_components = n_components;
//
//    type_comp_list_t c_list = components;
//    struct type_component *type_components = type_node->type.components;
//
//    while (c_list) {
//        *type_components++ = c_list->component;
//        type_comp_list_t next = c_list->tail;
//        free(c_list);
//        c_list = next;
//    }
//
//    // Add the function to the context
//    type_node->tail = context->types;
//    context->types = type_node;
//
//    return 0;
}