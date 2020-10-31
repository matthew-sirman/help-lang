//
// Created by matthew on 23/10/2020.
//

#ifndef FUNCTIONAL_PARSER_H
#define FUNCTIONAL_PARSER_H

#include <glob.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../lang/context.h"

// TODO: Token extraction method (rather than repeating code!)

typedef enum token { NONE = 0, FUNC, TYPE, ERROR } token_t;

typedef enum parse_error { SUCCESS, INCOMPLETE_DECLARATION, MALFORMED_FUNC, MALFORMED_TYPE } parse_error_t;

//typedef struct type_comp_node *type_comp_list_t;

//struct type_comp_node {
//    struct type_component component;
//    type_comp_list_t tail;
//};

// External functions

// Parses source code and writes all functions to the context builder, and all expressions to the expression manager
parse_error_t parse_source(context_t *context, expr_manager_t *expr_manager, const char *source, size_t length);

// Internal functions

// Finds the next function declaration in the string region between s_begin and s_end. Writes the extracted region
// to r_begin and r_end. Returns 0 if successful, otherwise 1 indicating failure to parse.
static token_t extract_declaration(const char *s_begin, const char *s_end, const char **r_begin, const char **r_end);

// Parse a single function
static int parse_function(context_t *context, expr_manager_t *expr_manager, const char *f_begin, const char *f_end);

// Parse an expression
static expr_ptr_t parse_function_expression(context_t *context, function_builder_t *f_builder,
                                            expr_manager_t *expr_manager, const char **e_begin, const char *e_end);

// Parse a single term in an expression
static expr_ptr_t parse_function_term(context_t *context, function_builder_t *f_builder,
                                      expr_manager_t *expr_manager, const char **e_begin, const char *e_end);

// Parse a single type
static int parse_type(context_t *context, const char *t_begin, const char *t_end);

#endif //FUNCTIONAL_PARSER_H
