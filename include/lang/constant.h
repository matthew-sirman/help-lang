//
// Created by matthew on 23/10/2020.
//

#ifndef FUNCTIONAL_LITERAL_H
#define FUNCTIONAL_LITERAL_H

#include "type.h"

typedef struct constant constant_t;
typedef void *value_t;

struct constant {
    value_t value;
    type_t *type;
};

constant_t *create_constant(value_t *value, type_t *type);

#endif //FUNCTIONAL_LITERAL_H
