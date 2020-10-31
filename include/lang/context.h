//
// Created by matthew on 23/10/2020.
//

#ifndef FUNCTIONAL_CONTEXT_H
#define FUNCTIONAL_CONTEXT_H

#include <stdlib.h>

#include "func.h"
#include "builtin.h"

typedef struct context_builder context_builder_t;
typedef struct func_node *func_list_t;
typedef struct base_type_node *base_type_list_t;
typedef struct type_node *type_list_t;
typedef struct context context_t;

struct context_builder {
    func_list_t functions;
    base_type_list_t base_types;
    type_list_t types;
    size_t n_functions, n_base_types, n_types;
};

struct func_node {
    func_t function;
    func_list_t tail;
};

struct base_type_node {
    base_type_t base_type;
    base_type_list_t tail;
};

struct type_node {
    type_t type;
    type_list_t tail;
};

struct context {
    func_t *functions;
    base_type_t *base_types;
    type_t *types;
    size_t n_functions, n_base_types, n_types;
};

context_builder_t *create_context_builder(void);

void free_context_builder(context_builder_t *builder);

func_t *builder_create_function(context_builder_t *builder);

base_type_t *get_builder_base_type(context_builder_t *builder, const char *name, size_t name_length);

type_t *get_builder_type(context_builder_t *builder, const char *name, size_t name_length);

type_t *builder_create_base_type(context_builder_t *builder, const char *name, size_t name_length);

type_t *builder_create_base_type_with_print(context_builder_t *builder, const char *name, size_t name_length, print_method_t print_method);

type_t *builder_create_function_type(context_builder_t *builder, type_t *from, type_t *to);

void remap_types(expr_t *expr, type_t *type_map);

context_t *compile_context(context_builder_t *builder);

base_type_t *get_base_type(context_t *context, const char *name, size_t name_length);

type_t *get_type(context_t *context, const char *name, size_t name_length);

void free_context(context_t *context);

#endif //FUNCTIONAL_CONTEXT_H
