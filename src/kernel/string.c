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

int32_t strcmp(const char* string1, const char* string2) {
    size_t str1Len = strlen(string1);
    size_t str2Len = strlen(string2);
    size_t minLen = str1Len > str2Len ? str2Len : str1Len;

    for (uint32_t i = 0; i < minLen; i++) {
        if (string1[i] != string2[i])
            return string1[i] - string2[i];
    }
    
    return str1Len - str2Len;
}