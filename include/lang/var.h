//
// Created by matthew on 25/10/2020.
//

#ifndef FUNC_VAR_H
#define FUNC_VAR_H

#include <stdlib.h>

#include "type.h"

typedef struct var var_t;
typedef struct expr expr_t;

struct var {
    char *name;
    size_t binding_index;
    type_t *type;
};

void set_variable(var_t *var, const char *name, size_t name_length, type_t *type);

#endif //FUNC_VAR_H
