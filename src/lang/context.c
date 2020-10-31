//
// Created by matthew on 23/10/2020.
//

#include <string.h>
#include "../../include/util.h"
#include "../../include/lang/context.h"

context_builder_t *create_context_builder(void) {
    context_builder_t *builder = (context_builder_t *) malloc(sizeof(context_builder_t));
    builder->functions = NULL;
    builder->base_types = NULL;
    builder->types = NULL;
    builder->n_functions = 0;
    builder->n_base_types = 0;
    builder->n_types = 0;
    return builder;
}

void free_context_builder(context_builder_t *builder) {
    func_list_t fl = builder->functions;

    while (fl) {
        // TODO: Replace with "free_func"
        func_list_t next = fl->tail;
        free(fl);
        fl = next;
    }
    base_type_list_t btl = builder->base_types;

    while (btl) {
        base_type_list_t next = btl->tail;
        free(btl);
        btl = next;
    }

    type_list_t tl = builder->types;

    while (tl) {
        type_list_t next = tl->tail;
        free(tl);
        tl = next;
    }

    free(builder);
}

func_t *builder_create_function(context_builder_t *builder) {
    func_list_t node = (func_list_t) malloc(sizeof(struct func_node));
    node->tail = builder->functions;
    builder->functions = node;
    builder->n_functions++;
    return &node->function;
}

base_type_t *get_builder_base_type(context_builder_t *builder, const char *name, size_t name_length) {
    // Search the list for a type with the matching name
    base_type_list_t btl = builder->base_types;

    while (btl) {
        size_t base_name_length = strlen(btl->base_type.name);
        if (base_name_length == name_length && string_equal(name, btl->base_type.name, name_length)) {
            return &btl->base_type;
        }

        btl = btl->tail;
    }
    // Return NULL if we don't find a matching type
    return NULL;
}

type_t *get_builder_type(context_builder_t *builder, const char *name, size_t name_length) {
    base_type_t *base_type = get_builder_base_type(builder, name, name_length);
    if (base_type) {
        type_list_t tl = builder->types;

        while (tl) {
            if (tl->type.mode == BASE_TYPE) {
                if (tl->type.type.base == base_type) {
                    return &tl->type;
                }
            }
            tl = tl->tail;
        }
    }
    return NULL;
}

type_t *builder_create_base_type(context_builder_t *builder, const char *name, size_t name_length) {
    // First check if the type already exists. If it does, return NULL to indicate that this is not an error,
    // but not the type itself as this could cause accidental changes to the base type.
    base_type_list_t btl = builder->base_types;

    while (btl) {
        size_t base_name_length = strlen(btl->base_type.name);
        if (base_name_length == name_length && string_equal(name, btl->base_type.name, name_length)) {
            return NULL;
        }

        btl = btl->tail;
    }

    // Create the base type
    base_type_list_t node = (base_type_list_t) malloc(sizeof(struct base_type_node));
    node->tail = builder->base_types;
    builder->base_types = node;
    builder->n_base_types++;
    node->base_type.name = (char *) malloc(sizeof(char) * (name_length + 1));
    memcpy(node->base_type.name, name, name_length);
    node->base_type.name[name_length] = '\0';
    node->base_type.print_method = NULL;

    // Create the type of the base type
    type_list_t t_node = (type_list_t) malloc(sizeof(struct type_node));
    t_node->tail = builder->types;
    builder->types = t_node;
    builder->n_types++;

    t_node->type.mode = BASE_TYPE;
    t_node->type.type.base = &node->base_type;

    return &t_node->type;
}

type_t *builder_create_base_type_with_print(context_builder_t *builder, const char *name, size_t name_length, print_method_t print_method) {
    type_t *type = builder_create_base_type(builder, name, name_length);
    type->type.base->print_method = print_method;
    return type;
}

type_t *builder_create_function_type(context_builder_t *builder, type_t *from, type_t *to) {
    // First check if the type already exists. If it does, return it as is.
    type_list_t tl = builder->types;

    while (tl) {
        if (tl->type.mode == FUNCTION_TYPE) {
            if (tl->type.type.function.from == from && tl->type.type.function.to == to) {
                return &tl->type;
            }
        }
        tl = tl->tail;
    }

    // Otherwise, construct a new node on the list
    type_list_t node = (type_list_t) malloc(sizeof(struct type_node));
    node->tail = builder->types;
    builder->types = node;
    builder->n_types++;

    // Set the type up based on the inputs
    node->type.mode = FUNCTION_TYPE;
    node->type.type.function.from = from;
    node->type.type.function.to = to;

    // Return the new type
    return &node->type;
}

void remap_types(expr_t *expr, type_t *type_map) {
    expr->type = &type_map[expr->type->type_id];
    switch (expr->mode) {
        case VARIABLE:
            expr->e.var->type = &type_map[expr->e.var->type->type_id];
            break;
        case ABSTRACTION:
            expr->e.lam->var.type = &type_map[expr->e.lam->var.type->type_id];
            remap_types(&expr->e.lam->expr, type_map);
            break;
        case APPLICATION:
            remap_types(expr->e.app.f, type_map);
            remap_types(expr->e.app.a, type_map);
            break;
        case FUNCTION:
            // Pass through to the entry point
            remap_types(expr->e.func->entry_point, type_map);
            break;
        case CONSTANT:
            expr->e.constant->type = &type_map[expr->e.constant->type->type_id];
            break;
    }
}

context_t *compile_context(context_builder_t *builder) {
    context_t *context = (context_t *) malloc(sizeof(context_t));

    context->n_functions = builder->n_functions;
    context->n_base_types = builder->n_base_types;
    context->n_types = builder->n_types;
    context->functions = (func_t *) malloc(sizeof(func_t) * context->n_functions);
    context->base_types = (base_type_t *) malloc(sizeof(base_type_t) * context->n_base_types);
    context->types = (type_t *) malloc(sizeof(type_t) * context->n_types);

    func_list_t fl = builder->functions;
    size_t i = 0;

    while (fl) {
        context->functions[i++] = fl->function;
        fl = fl->tail;
    }

    base_type_list_t btl = builder->base_types;
    i = 0;

    while (btl) {
        btl->base_type.base_type_id = i;
        context->base_types[i++] = btl->base_type;
        btl = btl->tail;
    }

    type_list_t tl = builder->types;
    i = 0;

    while (tl) {
        tl->type.type_id = i;
        context->types[i++] = tl->type;
        tl = tl->tail;
    }

    for (type_t *type = context->types; type < &context->types[context->n_types]; type++) {
        switch (type->mode) {
            case BASE_TYPE:
                type->type.base = &context->base_types[type->type.base->base_type_id];
                break;
            case FUNCTION_TYPE:
                type->type.function.from = &context->types[type->type.function.from->type_id];
                type->type.function.to = &context->types[type->type.function.to->type_id];
                break;
        }
    }

    for (func_t *func = context->functions; func < &context->functions[context->n_functions]; func++) {
        func->lambdas[0].var.type = &context->types[func->lambdas[0].var.type->type_id];
        func->func_expr->e.func = func;
        remap_types(&func->lambdas[0].expr, context->types);
    }

    return context;
}

base_type_t *get_base_type(context_t *context, const char *name, size_t name_length) {
    for (base_type_t *base = context->base_types; base < context->base_types + context->n_base_types; base++) {
        size_t base_name_length = strlen(base->name);
        if (base_name_length == name_length && string_equal(name, base->name, name_length)) {
            return base;
        }
    }
    return NULL;
}

type_t *get_type(context_t *context, const char *name, size_t name_length) {
    for (type_t *type = context->types; type < context->types + context->n_types; type++) {
        if (type->mode == BASE_TYPE) {
            size_t base_name_length = strlen(type->type.base->name);
            if (base_name_length == name_length && string_equal(name, type->type.base->name, name_length)) {
                return type;
            }
        }
    }
    return NULL;
}

void free_context(context_t *context) {
    free(context->functions);
    free(context->base_types);
    free(context->types);
    free(context);
}
