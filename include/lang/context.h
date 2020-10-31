//
// Created by matthew on 23/10/2020.
//

#ifndef FUNCTIONAL_CONTEXT_H
#define FUNCTIONAL_CONTEXT_H

#include <stdlib.h>

#include "func.h"
#include "builtin.h"

//typedef struct context_builder context_builder_t;
//typedef struct func_node *func_list_t;
//typedef struct base_type_node *base_type_list_t;
//typedef struct type_node *type_list_t;
typedef struct context context_t;

struct context {
//    func_list_t functions;
//    base_type_list_t base_types;
//    type_list_t types;
    func_t *functions;
    base_type_t *base_types;
    type_t *types;

    size_t n_functions, n_base_types, n_types;
    size_t func_capacity, base_type_capacity, type_capacity;
};

//struct func_node {
//    func_t function;
//    func_list_t tail;
//};
//
//struct base_type_node {
//    base_type_t base_type;
//    base_type_list_t tail;
//};
//
//struct type_node {
//    type_t type;
//    type_list_t tail;
//};

//struct context {
//    func_t *functions;
//    base_type_t *base_types;
//    type_t *types;
//    size_t n_functions, n_base_types, n_types;
//};

context_t *create_context(void);

func_ptr_t context_create_function(context_t *context);

func_ptr_t context_find_function(context_t *context, const char *name, size_t name_length);

//base_type_ptr_t get_builder_base_type_ptr(context_builder_t *builder, const char *name, size_t name_length);
//
//type_ptr_t get_builder_type_ptr(context_builder_t *builder, const char *name, size_t name_length);
//
//base_type_t *get_builder_base_type(context_builder_t *builder, base_type_ptr_t base);
//
//type_t *get_builder_type(context_builder_t *builder, type_ptr_t type);

type_ptr_t context_create_base_type(context_t *context, const char *name, size_t name_length);

type_ptr_t context_create_base_type_with_print(context_t *context, const char *name, size_t name_length, print_method_t print_method);

type_ptr_t context_create_function_type(context_t *context, type_ptr_t from, type_ptr_t to);

//void remap_types(expr_t *expr, type_t *type_map);

//context_t *compile_context(context_builder_t *builder);

base_type_ptr_t get_base_type_ptr(context_t *context, const char *name, size_t name_length);

type_ptr_t get_type_ptr(context_t *context, const char *name, size_t name_length);

func_ptr_t get_function_ptr(context_t *context, const char *name, size_t name_length);

func_t *get_function(context_t *context, func_ptr_t func);

base_type_t *get_base_type(context_t *context, base_type_ptr_t base_type);

type_t *get_type(context_t *context, type_ptr_t type);

void free_context(context_t *context);

#endif //FUNCTIONAL_CONTEXT_H
