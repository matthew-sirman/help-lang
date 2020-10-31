//
// Created by matthew on 25/10/2020.
//

#ifndef FUNC_VAR_H
#define FUNC_VAR_H

#include <stdlib.h>

#include "type.h"

// Typedefs from other files
typedef struct expr_manager expr_manager_t;

typedef struct var var_t;
typedef size_t var_ptr_t;

struct var {
    char *name;
    size_t binding_index;
    type_ptr_t type;
};

var_ptr_t create_variable(expr_manager_t *manager);

void set_variable(expr_manager_t *manager, var_ptr_t var, const char *name, size_t name_length, type_ptr_t type);

#endif //FUNC_VAR_H
