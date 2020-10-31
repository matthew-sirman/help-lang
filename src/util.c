//
// Created by matthew on 23/10/2020.
//

#include "../include/util.h"

int string_equal(const char *s1, const char *s2, size_t scan) {
    while (scan--) {
        if (*s1++ != *s2++) {
            return 0;
        }
    }
    return 1;
}