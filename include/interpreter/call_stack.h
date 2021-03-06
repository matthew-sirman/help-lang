//
// Created by matthew on 29/10/2020.
//

#ifndef HELP_LANG_CALL_STACK_H
#define HELP_LANG_CALL_STACK_H

#include "../lang/expr.h"

#define SIZE_DROP_THRESHOLD 4
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

typedef struct call_stack call_stack_t;
typedef struct stack_node stack_node_t;
typedef stack_node_t *call_stack_ptr;
typedef struct { size_t size, remaining; } working_set_t;

struct stack_node {
    expr_ptr_t expr;
    size_t stack_offset;
};

struct call_stack {
    call_stack_ptr stack_mem;
    call_stack_ptr stack_ptr;
    size_t capacity;
    size_t top_offset;
    working_set_t *working_sets;
    size_t working_set_capacity;
    size_t working_set_top;
};

// Create a call stack with initially a single space
call_stack_t *create_call_stack(void);

// Push frame_size empty items onto the stack, i.e. to allocate memory for function variables
void call_stack_push_frame(call_stack_t *stack, size_t frame_size);

// Emplace an item on the stack
void call_stack_emplace_expr(call_stack_t *stack, size_t index, expr_ptr_t expr, size_t stack_offset);

// Notify that a free variable in the current stack frame has completed. When all have completed,
// the stack frame can be freed
void call_stack_notify_pop(call_stack_t *stack, size_t frame);

// Get the call stack pointer
call_stack_ptr get_call_stack_ptr(call_stack_t *stack);

// Get the offset of the current stack pointer
size_t get_call_stack_offset(call_stack_t *stack);

// Get the size of the active stack frame
size_t get_stack_frame_size(call_stack_t *stack);

// Get the current stack frame
size_t get_stack_frame(call_stack_t *stack);

// Free the call stack
void free_call_stack(call_stack_t *stack);

// Update the capacity on the stack
static void update_capacity(call_stack_t *stack, size_t new_capacity);

//// Decrease the capacity on the stack
//static void decrease_capacity(call_stack_t *stack, size_t new_capacity);

// Update the capacity of the working set
static void update_working_set_capacity(call_stack_t *stack, size_t new_capacity);

//// Halve the capacity of the working set
//static void halve_working_set_capacity(call_stack_t *stack);

#endif //HELP_LANG_CALL_STACK_H
