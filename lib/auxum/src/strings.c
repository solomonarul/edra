#include "strings.h"

#include <string.h>

void string_concat(char* const dest, char* const first, char* const second)
{
    const int first_len = strlen(first);
    const int second_len = strlen(second);
    _memccpy((void*)dest, first, '\0', first_len);
    _memccpy((void*)(dest + first_len), second, '\0', second_len);
    dest[first_len + second_len] = '\0';
}
