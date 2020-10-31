//
// Created by matthew on 26/10/2020.
//

#include <string.h>
#include "../../include/lang/var.h"

void set_variable(var_t *var, const char *name, size_t name_length, type_t *type) {
    if (name) {
        var->name = (char *) malloc(sizeof(char) * (name_length + 1));
        memcpy(var->name, name, name_length);
        var->name[name_length] = '\0';
    } else {
        var->name = NULL;
    }
    var->type = type;
}