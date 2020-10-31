//
// Created by matthew on 28/10/2020.
//

#include <stdlib.h>
#include "../../include/lang/constant.h"

constant_t *create_constant(value_t *value, type_t *type) {
    constant_t *c = (constant_t *) malloc(sizeof(constant_t));
    c->value = value;
    c->type = type;
    return c;
}
