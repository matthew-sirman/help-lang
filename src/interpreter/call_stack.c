//
// Created by matthew on 29/10/2020.
//

#include <stdio.h>
#include <string.h>
#include "../../include/interpreter/call_stack.h"

call_stack_t *create_call_stack(void) {
    // Create the stack
    call_stack_t *stack = (call_stack_t *) malloc(sizeof(call_stack_t));
    // Initialise the capacity to 1 and offset to 0
    stack->capacity = 1;
    stack->top_offset = 0;
    // Allocate a pointer for the stack memory of size 1 and set the stack pointer to NULL
    stack->stack_mem = (call_stack_ptr) malloc(sizeof(stack_node_t));
    stack->stack_ptr = NULL;
    // Setup working set structure
    stack->working_set_capacity = 1;
    stack->working_set_top = 0;
    stack->working_sets = (working_set_t *) malloc(sizeof(working_set_t));
    stack->working_sets[0].size = 0;
    stack->working_sets[0].remaining = 0;
    return stack;
}

void call_stack_push_frame(call_stack_t *stack, size_t frame_size) {
    if (!frame_size) {
        fprintf(stderr, "Attempted to push an empty stack frame.\n");
        return;
    }

    size_t new_size = stack->top_offset + frame_size;
    if (new_size > stack->capacity) {
        // If the size will exceed the capacity, we double the capacity until it is large enough
        size_t new_stack_size = stack->capacity;
        while (new_stack_size < new_size) {
            new_stack_size *= 2;
        }
        update_capacity(stack, new_stack_size);
    }
    // Set each pointer to NULL to initialise
    for (stack_node_t *e = stack->stack_mem + stack->top_offset; e < stack->stack_mem + new_size; e++) {
        e->expr = NULL_PTR;
        e->stack_offset = 0;
    }
    // Set the stack pointer to the base of this frame
    stack->stack_ptr = stack->stack_mem + stack->top_offset;
    // Update the stack offset
    stack->top_offset = new_size;

    // Increase the working set size if necessary
    if (stack->working_set_top + 1 > stack->working_set_capacity) {
        update_working_set_capacity(stack, stack->working_set_capacity * 2);
    }
    // Set the new working set to the stack frame pushed
    stack->working_sets[stack->working_set_top].size = frame_size;
    stack->working_sets[stack->working_set_top].remaining = frame_size;
    // Increment the top offset
    stack->working_set_top++;
}


void call_stack_emplace_expr(call_stack_t *stack, size_t index, expr_ptr_t expr, size_t stack_offset) {
    stack->stack_ptr[index].expr = expr;
    stack->stack_ptr[index].stack_offset = stack_offset;
}

void call_stack_notify_pop(call_stack_t *stack, size_t frame) {
    if (stack->working_set_top == 0) {
        fprintf(stderr, "Attempted to pop from empty stack.\n");
    }

    // Decrement the remaining counter for the stack frame.
    stack->working_sets[frame].remaining--;
    // If the stack frame was the top, we can pop any empty stack frames
    if (frame == stack->working_set_top - 1) {
        size_t new_stack_size = stack->top_offset;
        // While the top stack frame has 0 remaining
        while (stack->working_sets[stack->working_set_top - 1].remaining == 0) {
            size_t ws_size = stack->working_sets[--stack->working_set_top].size;
            new_stack_size -= ws_size;
        }

        if (stack->top_offset != new_stack_size) {
            if (new_stack_size * SIZE_DROP_THRESHOLD < stack->capacity) {
                size_t new_stack_capacity = stack->capacity;
                while (new_stack_size * SIZE_DROP_THRESHOLD < new_stack_capacity) {
                    new_stack_capacity /= 2;
                }
                if (new_stack_capacity == 0) {
                    new_stack_capacity = 1;
                }
                update_capacity(stack, new_stack_capacity);
            }
            if (stack->working_set_top * SIZE_DROP_THRESHOLD < stack->working_set_capacity) {
                size_t new_working_set_capacity = stack->working_set_capacity;
                while (stack->working_set_top * SIZE_DROP_THRESHOLD < new_working_set_capacity) {
                    new_working_set_capacity /= 2;
                }
                if (new_working_set_capacity == 0) {
                    new_working_set_capacity = 1;
                }
                update_working_set_capacity(stack, new_working_set_capacity);
            }
            stack->top_offset = new_stack_size;
            // Update the stack base pointer
            if (stack->working_set_top == 0) {
                stack->stack_ptr = NULL;
            } else {
                stack->stack_ptr =
                        stack->stack_mem + stack->top_offset - stack->working_sets[stack->working_set_top - 1].size;
            }
        }
    }

//        // First, decrement the working_set_top variable
//        stack->working_set_top--;
//        // Next get the size of the stack frame we are popping
//        size_t ws_size = stack->working_sets[stack->working_set_top].size;
//        // If we need to shrink the working sets, do so
//        if (stack->working_set_top * SIZE_DROP_THRESHOLD < stack->working_set_capacity) {
//            halve_working_set_capacity(stack);
//        }
//        // Next, pop ws_size elements from the stack
//        size_t new_size = stack->top_offset - ws_size;
//        // If we need to decrease the stack size
//        if (new_size * SIZE_DROP_THRESHOLD < stack->capacity) {
//            // Initialise a value to the stack capacity
//            size_t new_stack_size = stack->capacity;
//            // Keep halving until the stack is small enough
//            while (new_size * SIZE_DROP_THRESHOLD < new_stack_size) {
//                new_stack_size /= 2;
//            }
//            // If we happened to halve to 0, don't decrease the capacity
//            if (new_stack_size != 0) {
//                // Decrease the capacity
//                update_capacity(stack, new_stack_size);
//            }
//        }
//        // Update the stack
//        stack->top_offset = new_size;
//        // Update the stack base pointer
//        if (stack->working_set_top == 0) {
//            stack->stack_ptr = NULL;
//        } else {
//            stack->stack_ptr = stack->stack_mem + stack->top_offset - stack->working_sets[stack->working_set_top - 1].size;
//        }

}

call_stack_ptr get_call_stack_ptr(call_stack_t *stack) {
    if (!stack->stack_ptr) {
        fprintf(stderr, "Attempted to access stack pointer for empty stack.\n");
        exit(1);
    }
    return stack->stack_ptr;
}

size_t get_call_stack_offset(call_stack_t *stack) {
    if (!stack->stack_ptr) {
        return -1;
    }
    return stack->stack_ptr - stack->stack_mem;
}

size_t get_stack_frame_size(call_stack_t *stack) {
    if (!stack->stack_ptr) {
        fprintf(stderr, "Attempted to retrieve stack frame size for empty stack.\n");
        exit(1);
    }
    return stack->working_sets[stack->working_set_top - 1].size;
}

size_t get_stack_frame(call_stack_t *stack) {
    return stack->working_set_top - 1;
}

void free_call_stack(call_stack_t *stack) {
    free(stack->stack_mem);
    free(stack->working_sets);
    free(stack);
}

void update_capacity(call_stack_t *stack, size_t new_capacity) {
    size_t stack_ptr_offset = stack->stack_ptr - stack->stack_mem;
    stack->capacity = new_capacity;
    stack->stack_mem = (call_stack_ptr) realloc((void *) stack->stack_mem, sizeof(stack_node_t) * new_capacity);
    if (stack->stack_ptr) {
        stack->stack_ptr = stack->stack_mem + stack_ptr_offset;
    }
}

void update_working_set_capacity(call_stack_t *stack, size_t new_capacity) {
    // Double the capacity
    stack->working_set_capacity = new_capacity;
    stack->working_sets = (working_set_t *) realloc((void *) stack->working_sets,
                                                    sizeof(working_set_t) * stack->working_set_capacity);
}

//void halve_working_set_capacity(call_stack_t *stack) {
//    // If the capacity is 1, simply return
//    if (stack->working_set_capacity == 1) {
//        return;
//    }
//    // Halve the capacity
//    stack->working_set_capacity /= 2;
//    stack->working_sets = (working_set_t *) realloc((void *) stack->working_sets, stack->working_set_capacity);
//}