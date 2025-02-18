#pragma once
#ifndef AUXUM_INI_H
#define AUXUM_INI_H

#include "dynarray.h"
#include <stdio.h>

#define MAX_LINE_WIDTH 255

struct ini_file {
    dynarray_t sections;
};
typedef struct ini_file ini_file_t;

struct ini_section{
    char* key;
    dynarray_t values;
};
typedef struct ini_section ini_section_t;

struct ini_data{
    enum {
        NONE = (uint8_t)0,
        ARRAY,
        VALUE,
        COUNT
    } type;

    union {
        char* string;       // For values.
        dynarray_t array;   // For arrays, should allow nesting.
    } data;
};
typedef struct ini_data ini_data_t;

struct ini_value {
    char* key;
    ini_data_t value;
};
typedef struct ini_value ini_value_t;

ini_file_t ini_file_parse(FILE* file);
void ini_file_print(ini_file_t* self, FILE* file);
void ini_file_free(ini_file_t* self);

ini_section_t* ini_file_get_section(ini_file_t* self, char* const key);
ini_data_t* ini_file_get_data(ini_file_t* self, char* const section, char* const key);
ini_data_t* ini_section_get_data(ini_section_t* self, char* const key);
char* ini_data_get_as_string(ini_data_t* self);
int ini_data_get_as_int(ini_data_t* self);

#endif
