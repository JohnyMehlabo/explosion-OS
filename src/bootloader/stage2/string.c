#include "string.h"
#include "stdint.h"
#include "stddef.h"

size_t strlen(const char* string) {
    uint32_t len = 0;
    while (string[len]) {
        len++;
    }
    return len;
}

char* strchr(const char *string, int c) {
    while (*string) {
        if (*string == c) return (char*)string;
        string++;
    }
    if (c == 0) return (char*)string;
    return (char*)NULL;
}