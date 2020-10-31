//
// Created by matthew on 28/10/2020.
//

#include "../../include/compiler/preprocessor.h"

void preprocess(const char *source, size_t source_length, char **preprocessed, size_t *preprocessed_size) {
    const char *end = source + source_length;
    size_t p_size = 0;
    for (const char *c = source; c < end; c++) {
        if (c < end - 1) {
            // If this is a comment, skip to the end of the line
            if (*c == '/' && *(c + 1) == '/') {
                do {
                    c++;
                } while (*c != '\n' && c < end);
            }
        }
        // Increment the size counter if the character isn't a newline
        p_size += (*c != '\n');
    }

    // Set the preprocessed size parameter to the truncated size
    *preprocessed_size = p_size;

    // Allocate enough memory for the truncated string
    *preprocessed = (char *) malloc(sizeof(char) * (p_size + 1));
    // Get a pointer to the character we wish to write to (starts at the beginning)
    char *write_char = *preprocessed;

    for (const char *c = source; c < end; c++) {
        if (c < end - 1) {
            // If this is a comment, skip over the line
            if (*c == '/' && *(c + 1) == '/') {
                do {
                    c++;
                } while (*c != '\n' && c < end);
            }
        }
        // Otherwise, if this isn't a newline, copy the character into the output buffer, and then
        // increment the buffer position
        if (*c != '\n') {
            *write_char++ = *c;
        }
    }

    // Set the final character to a null terminator
    *write_char = '\0';
}
