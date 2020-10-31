//
// Created by matthew on 30/10/2020.
//

#ifndef HELP_LANG_STRING_BUFFER_H
#define HELP_LANG_STRING_BUFFER_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct string_buffer string_buffer_t;

struct string_buffer {
    char *buffer;
    size_t capacity;
    size_t used;
};

string_buffer_t *create_string_buffer(size_t start_capacity);

void append_string(string_buffer_t *buffer, const char *string, size_t len);

const char *get_string_start(string_buffer_t *buffer);

const char *get_string_end(string_buffer_t *buffer);

bool buffer_empty(string_buffer_t *buffer);

void reset_buffer(string_buffer_t *buffer);

void free_string_buffer(string_buffer_t *buffer);

#endif //HELP_LANG_STRING_BUFFER_H
