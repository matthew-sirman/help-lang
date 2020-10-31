//
// Created by matthew on 23/10/2020.
//

#ifndef FUNCTIONAL_LITERAL_H
#define FUNCTIONAL_LITERAL_H

#include "type.h"

// Typedefs for other files
typedef struct expr_manager expr_manager_t;

typedef struct constant constant_t;
typedef size_t constant_ptr_t;
typedef void *value_t;

struct constant {
    value_t value;
    type_ptr_t type;
};

constant_ptr_t create_constant(expr_manager_t *manager, value_t *value, type_ptr_t type);

#endif //FUNCTIONAL_LITERAL_H
