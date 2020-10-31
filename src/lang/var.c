//
// Created by matthew on 26/10/2020.
//

#include <string.h>
#include "../../include/lang/expr.h"
#include "../../include/lang/var.h"

var_ptr_t create_variable(expr_manager_t *manager) {
    if (manager->n_variables + 1 > manager->var_capacity) {
        manager->var_capacity *= 2;
        manager->variables = (var_t *) realloc(manager->variables, sizeof(var_t) * manager->var_capacity);
    }
    return ++manager->n_variables;
}

void set_variable(expr_manager_t *manager, var_ptr_t var, const char *name, size_t name_length, type_ptr_t type) {
    var_t *p_var = get_variable(manager, var);
    if (name) {
        p_var->name = (char *) malloc(sizeof(char) * (name_length + 1));
        memcpy(p_var->name, name, name_length);
        p_var->name[name_length] = '\0';
    } else {
        p_var->name = NULL;
    }
    p_var->type = type;
}