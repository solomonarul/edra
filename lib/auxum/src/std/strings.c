#include "std/strings.h"

#include <string.h>

void string_concat(char* const dest, char* const first, char* const second)
{
    const int first_len = strlen(first);
    const int second_len = strlen(second);
    memccpy((void*)dest, first, '\0', first_len);
    memccpy((void*)(dest + first_len), second, '\0', second_len);
    dest[first_len + second_len] = '\0';
}

char* string_strip(char* const src)
{
    char* result = src;
    while((result[0] == ' ' || result[0] == '\n') && result[0] != '\0') result++;
    int length = strlen(result);
    while((result[length - 1] == ' ' || result[length - 1] == '\n') && length > 0)
    {
        result[length - 1] = '\0';
        length--;
    }
    return result;
}
