//
// Created by matthew on 23/10/2020.
//

#ifndef FUNCTIONAL_INTERPRETER_H
#define FUNCTIONAL_INTERPRETER_H

#include <stdio.h>

#include "../lang/context.h"
#include "call_stack.h"

typedef enum { CMD_INVALID, CMD_EXIT } cmd_code_t;

// Enter the REPL interpreter
void start_repl_interpreter(context_t *context, expr_manager_t *expr_manager);

static cmd_code_t handle_command(const char *cmd, size_t len);

// Executes a given function with the provided arguments
static void execute_statement(context_t *context, expr_manager_t *expr_manager, const char *stmt_begin, const char *stmt_end);

static expr_t *parse_expression(context_t *context, expr_manager_t *expr_manager, const char **e_begin, const char *e_end);

static expr_t *parse_expression_term(context_t *context, expr_manager_t *expr_manager, const char **e_begin, const char *e_end);

//void execute_function(func_t *f, );

expr_t *evaluate(expr_manager_t *manager, expr_t *e);

expr_t *evaluate_expr(expr_manager_t *manager, expr_t *e, call_stack_t *call_stack);

//expr_t *execute_function(func_t *func, )

void print_type(type_t *type);

void print_value(constant_t *val);

void print_expr_component(expr_t *e);

void print_expr(expr_t *e);

#endif //FUNCTIONAL_INTERPRETER_H
