#include "string.h"

/**
 * Minimal implementation of strncpy
 * @param dest: destination where the new string will reside
 * @param src: the source string
 * @param n: size of the destination string
 * @return the copied string
 */
char *my_strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for(i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for(; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}