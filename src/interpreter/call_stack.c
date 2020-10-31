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
    stack->stack_mem = (call_stack_ptr) malloc(sizeof(expr_t *));
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
    size_t new_size = stack->top_offset + frame_size;
    if (new_size > stack->capacity) {
        // If the size will exceed the capacity, we double the capacity until it is large enough
        size_t new_stack_size = stack->capacity;
        while (new_stack_size < new_size) {
            new_stack_size *= 2;
        }
        increase_capacity(stack, new_stack_size);
    }
    // Set each pointer to NULL to initialise
    for (expr_t **e = stack->stack_mem + stack->top_offset; e < stack->stack_mem + new_size; e++) {
        *e = NULL;
    }
    // Set the stack pointer to the base of this frame
    stack->stack_ptr = stack->stack_mem + stack->top_offset;
    // Update the stack offset
    stack->top_offset = new_size;

    // Increase the working set size if necessary
    if (stack->working_set_top + 1 > stack->working_set_capacity) {
        double_working_set_capacity(stack);
    }
    // Set the new working set to the stack frame pushed
    stack->working_sets[stack->working_set_top].size = frame_size;
    stack->working_sets[stack->working_set_top].remaining = frame_size;
    // Increment the top offset
    stack->working_set_top++;
}

void call_stack_notify_pop(call_stack_t *stack) {
    if (stack->working_set_top == 0) {
        fprintf(stderr, "Attempted to pop from empty stack.\n");
    }

    // Decrement the remaining counter for the top stack frame. If it is now empty, we can pop the
    // frame.
    if (--stack->working_sets[stack->working_set_top - 1].remaining == 0) {
        // First, decrement the working_set_top variable
        stack->working_set_top--;
        // Next get the size of the stack frame we are popping
        size_t ws_size = stack->working_sets[stack->working_set_top].size;
        // If we need to shrink the working sets, do so
        if (stack->working_set_top * SIZE_DROP_THRESHOLD < stack->working_set_capacity) {
            halve_working_set_capacity(stack);
        }
        // Next, pop ws_size elements from the stack
        size_t new_size = stack->top_offset - ws_size;
        // If we need to decrease the stack size
        if (new_size * SIZE_DROP_THRESHOLD < stack->capacity) {
            // Initialise a value to the stack capacity
            size_t new_stack_size = stack->capacity;
            // Keep halving until the stack is small enough
            while (new_size * SIZE_DROP_THRESHOLD < new_stack_size) {
                new_stack_size /= 2;
            }
            // If we happened to halve to 0, don't decrease the capacity
            if (new_stack_size != 0) {
                // Decrease the capacity
                decrease_capacity(stack, new_stack_size);
            }
        }
        // Update the stack
        stack->top_offset = new_size;
        // Update the stack base pointer
        if (stack->working_set_top == 0) {
            stack->stack_ptr = NULL;
        } else {
            stack->stack_ptr = stack->stack_mem + stack->top_offset - stack->working_sets[stack->working_set_top].size;
        }
    }
}

call_stack_ptr get_call_stack_ptr(call_stack_t *stack) {
    if (!stack->stack_ptr) {
        fprintf(stderr, "Attempted to access stack pointer for empty stack.\n");
        exit(1);
    }
    return stack->stack_ptr;
}

size_t get_stack_frame_size(call_stack_t *stack) {
    if (!stack->stack_ptr) {
        fprintf(stderr, "Attempted to retrieve stack frame size for empty stack.\n");
        exit(1);
    }
    return stack->working_sets[stack->working_set_top - 1].size;
}

void free_call_stack(call_stack_t *stack) {
    free(stack->stack_mem);
    free(stack->working_sets);
    free(stack);
}

static void increase_capacity(call_stack_t *stack, size_t new_capacity) {
    call_stack_ptr new_stack = (call_stack_ptr) malloc(sizeof(expr_t *) * new_capacity);
    memcpy(new_stack, stack->stack_mem, sizeof(expr_t *) * stack->top_offset);
    free(stack->stack_mem);
    stack->stack_mem = new_stack;
    stack->capacity = new_capacity;
}

static void decrease_capacity(call_stack_t *stack, size_t new_capacity) {
    call_stack_ptr new_stack = (call_stack_ptr) malloc(sizeof(expr_t *) * new_capacity);
    memcpy(new_stack, stack->stack_mem, sizeof(expr_t *) * MIN(stack->top_offset, new_capacity));
    free(stack->stack_mem);
    stack->stack_mem = new_stack;
    stack->capacity = new_capacity;
}

static void double_working_set_capacity(call_stack_t *stack) {
    // Double the capacity
    stack->working_set_capacity *= 2;
    // Allocate memory for the new working sets
    working_set_t *new_working_sets = (working_set_t *) malloc(sizeof(working_set_t) * stack->working_set_capacity);
    memcpy(new_working_sets, stack->working_sets, sizeof(working_set_t) * stack->working_set_top);
    free(stack->working_sets);
    stack->working_sets = new_working_sets;
}

static void halve_working_set_capacity(call_stack_t *stack) {
    // If the capacity is 1, simply return
    if (stack->working_set_capacity == 1) {
        return;
    }
    // Halve the capacity
    stack->working_set_capacity /= 2;
    // Update the working sets
    working_set_t *new_working_sets = (working_set_t *) malloc(sizeof(working_set_t) * stack->working_set_capacity);
    memcpy(new_working_sets, stack->working_sets, sizeof(working_set_t) * MIN(stack->working_set_top, stack->working_set_capacity));
    free(stack->working_sets);
    stack->working_sets = new_working_sets;
}