#pragma once
#ifndef AUXUM_INI_H
#define AUXUM_INI_H

#include "../std/error.h"
#include "../data/dynarray.h"
#include <stdio.h>

#define MAX_LINE_WIDTH 255

struct ini_file {
    dynarray_t sections;
};
typedef struct ini_file ini_file_t;
DEFINE_RESULT(ini_file_t, char*, ini_file_result_t);
DEFINE_RESULT(int, char*, ini_int_result_t);
DEFINE_RESULT(bool, char*, ini_bool_result_t);
DEFINE_RESULT(char*, char*, ini_string_result_t);

struct ini_section{
    char* key;
    dynarray_t values;
};
typedef struct ini_section ini_section_t;
DEFINE_RESULT(ini_section_t*, char*, ini_section_ptr_result_t);

struct ini_data{
    enum {
        INI_DATA_NONE = (uint8_t)0,
        INI_DATA_ARRAY, INI_DATA_VALUE, INI_DATA_COUNT
    } type;

    union {
        char* string;       // For values.
        dynarray_t array;   // For arrays, should allow nesting.
    } data;
};
typedef struct ini_data ini_data_t;
DEFINE_RESULT(ini_data_t*, char*, ini_data_ptr_result_t);

struct ini_value {
    char* key;
    ini_data_t value;
};

typedef struct ini_value ini_value_t;

ini_file_result_t ini_file_read(char* const path);
ini_file_result_t ini_file_parse(FILE* file);
void ini_file_print(ini_file_t* self, FILE* file);
void ini_file_free(ini_file_t* self);

ini_section_ptr_result_t ini_file_get_section(ini_file_t* self, char* const key);
ini_data_ptr_result_t ini_file_get_data(ini_file_t* self, char* const section, char* const key);
ini_data_ptr_result_t ini_section_get_data(ini_section_t* self, char* const key);
int ini_data_get_array_size(ini_data_t* self);
ini_data_ptr_result_t ini_data_get_from_array(ini_data_t* self, uint32_t index);
ini_string_result_t ini_data_get_as_string(ini_data_t* self);
ini_int_result_t ini_data_get_as_int(ini_data_t* self);
ini_bool_result_t ini_data_get_as_bool(ini_data_t* self);

#endif
