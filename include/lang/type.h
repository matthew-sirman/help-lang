//
// Created by matthew on 23/10/2020.
//

#ifndef FUNCTIONAL_TYPE_H
#define FUNCTIONAL_TYPE_H

#define UNKNOWN_PRINT NULL

/* A type is defined as:
 * t : t -> t | T
 * Where T is a member of B, the fixed set of base types (which a user may
 * add to - the set is fixed at the time of typing an expression)
 * */

typedef struct type type_t;
typedef struct base_type base_type_t;
typedef enum { /* POLYMORPHIC_TYPE, */ BASE_TYPE, FUNCTION_TYPE } type_mode_t;
typedef void *value_t;
typedef void (*print_method_t)(value_t *);

//struct type_component {
//    char *name;
//    type_t *bound_type;
//};

struct base_type {
    char *name;
    print_method_t print_method;
    size_t base_type_id;
};

struct type {
    type_mode_t mode;
    union {
        base_type_t *base;
        struct {
            type_t *from, *to;
        } function;
    } type;
    size_t type_id;
};

#endif //FUNCTIONAL_TYPE_H
