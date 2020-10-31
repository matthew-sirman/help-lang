//
// Created by matthew on 28/10/2020.
//

#include <stdlib.h>
#include "../../include/lang/expr.h"
#include "../../include/lang/constant.h"

constant_ptr_t create_constant(expr_manager_t *manager, value_t *value, type_ptr_t type) {
    if (manager->n_constants + 1 > manager->const_capacity) {
        manager->const_capacity *= 2;
        manager->constants = (constant_t *) realloc(manager->constants, sizeof(constant_t) * manager->const_capacity);
    }
    constant_t *c = &manager->constants[manager->n_constants++];
    c->value = value;
    c->type = type;
    return manager->n_constants;
}
