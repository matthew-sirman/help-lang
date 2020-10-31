//
// Created by matthew on 30/10/2020.
//

#include <string.h>
#include "../../include/interpreter/string_buffer.h"

string_buffer_t *create_string_buffer(size_t start_capacity) {
    string_buffer_t *buffer = (string_buffer_t *) malloc(sizeof(string_buffer_t));
    buffer->capacity = start_capacity;
    buffer->buffer = (char *) malloc(sizeof(char) * start_capacity);
    buffer->used = 0;
    return buffer;
}

void append_string(string_buffer_t *buffer, const char *string, size_t len) {
    size_t new_size = buffer->used + len;

    if (new_size + 1 > buffer->capacity) {
        size_t new_capacity = buffer->capacity;
        while (new_size + 1 > new_capacity) {
            new_capacity *= 2;
        }
        buffer->buffer = (char *) realloc((void *) buffer->buffer, new_capacity);
    }

    memcpy(buffer->buffer + buffer->used, string, len);
    buffer->used = new_size;
    buffer->buffer[buffer->used] = '\0';
}

const char *get_string_start(string_buffer_t *buffer) {
    return buffer->buffer;
}

const char *get_string_end(string_buffer_t *buffer) {
    return buffer->buffer + buffer->used;
}

bool buffer_empty(string_buffer_t *buffer) {
    return buffer->used == 0;
}

void reset_buffer(string_buffer_t *buffer) {
    buffer->used = 0;
    buffer->buffer[0] = 0;
}

void free_string_buffer(string_buffer_t *buffer) {
    free(buffer->buffer);
    free(buffer);
}