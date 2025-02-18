#include "ini.h"
#include <auxum/strings.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

static void ini_array_parse(ini_data_t* self, char* line)
{
    self->type = ARRAY;
    dynarray_init(&self->data.array, sizeof(ini_data_t), 0);
    line++;
    int length = strlen(line);
    line[--length] = '\0';
    while(length > 0)
    {
        line = string_strip(line);
        char* end = line;
        while(*end != '\0' && *end != ',') end++;
        *end = '\0';
        length = strlen(line);
        if(length == 0) break;
        ini_data_t other;
        if(line[0] == '[' && line[length - 1] == ']')
            ini_array_parse(&other, line);
        else
        {
            other.type = VALUE;
            other.data.string = _strdup(line);
        }
        dynarray_push_back(&self->data.array, &other);
        line = end + 1;
    }
}

ini_file_t ini_file_parse(FILE* file)
{
    // Create the array.
    ini_file_t result;
    dynarray_init(&result.sections, sizeof(ini_section_t), 1);

    // Default section for those without a section.
    ini_section_t* current_section = dynarray_get(result.sections, 0);
    current_section->key = NULL;

    // Read the file.
    int length;
    char lineBuffer[MAX_LINE_WIDTH + 1], *line;
    while(fgets(lineBuffer, MAX_LINE_WIDTH + 1, file) != NULL)
    {
        line = string_strip(lineBuffer);
        length = strlen(line);

        // Detect which thing are we reading.
        if(line[0] == '[' && line[length - 1] == ']')                   // We are reading a section.
        {
            ini_section_t new_section;
            line[length - 1] = '\0';
            new_section.key = _strdup(line + 1);
            dynarray_init(&new_section.values, sizeof(ini_value_t), 0);
            current_section = dynarray_push_back(&result.sections, &new_section);
        }
        else {                                                          // We are probably reading a key=value pair
            char* second_part = strchr(line, '=');
            if(second_part != NULL)
            {
                second_part[0] = '\0'; second_part++;
                line = string_strip(line);
                second_part = string_strip(second_part);
                length = strlen(line);
                ini_value_t new_value;
                new_value.key = _strdup(line);
                if(line[length - 1] == ']' && line[length - 2] == '[')  // This is an array.
                {
                    line[length - 2] = '\0';
                    new_value.key = _strdup(line);
                    ini_array_parse(&new_value.value, second_part);
                }
                else {                                                  // This is a normal key=value.
                    new_value.key = _strdup(line);
                    new_value.value.type = VALUE;
                    new_value.value.data.string = _strdup(second_part);
                }
                dynarray_push_back(&current_section->values, &new_value);
            }
        }
        // Ignore otherwise.
    }
    return result;
}

static void ini_data_print(ini_data_t* self, FILE* file)
{
    if(self->type == VALUE)
        fprintf(file, "%s", self->data.string);
    else
    {
        fprintf(file, "[");
        for(uint32_t index = 0; index < self->data.array.size; index++)
        {
            ini_data_t* value = dynarray_get(self->data.array, index);
            ini_data_print(value, file);
            if(index != self->data.array.size - 1)
                fprintf(file, ",");
        }
        fprintf(file, "]");
    }
}

void ini_file_print(ini_file_t* self, FILE* file)
{
    ini_section_t* current_section;
    ini_value_t* current_value;
    for(uint32_t index = 0; index < self->sections.size; index++)
    {
        current_section = dynarray_get(self->sections, index);
        if(current_section->key != NULL)
            fprintf(file, "[%s]\n", current_section->key);
        for(uint32_t index_section = 0; index_section < current_section->values.size; index_section++)
        {
            current_value = dynarray_get(current_section->values, index_section);
            if(current_value->value.type == ARRAY)
                fprintf(file, "%s[]=", current_value->key);
            else
                fprintf(file, "%s=", current_value->key);
            ini_data_print(&current_value->value, file);
            fprintf(file, "\n");
        }
    }
}

static void ini_data_free(void* data)
{
    ini_data_t* const self = (ini_data_t*) data;
    switch(self->type)
    {
    case ARRAY:
        self->data.array.free_callback = ini_data_free;
        dynarray_free(self->data.array);
        break;
    case VALUE:
        free(self->data.string);
        break;
    default:
        break;
    }
}

static void ini_value_free(void* value)
{
    ini_value_t* const self = (ini_value_t*) value;
    free(self->key);
    ini_data_free((void*)&self->value);
}

static void ini_section_free(void* section)
{
    ini_section_t* const self = (ini_section_t*) section;
    free(self->key);
    self->values.free_callback = ini_value_free;
    dynarray_free(self->values);
}

void ini_file_free(ini_file_t* self)
{
    self->sections.free_callback = ini_section_free;
    dynarray_free(self->sections);
}

ini_section_t* ini_file_get_section(ini_file_t* self, char* const key)
{
    for(uint32_t index = 0; index < self->sections.size; index++)
    {
        ini_section_t* const value = dynarray_get(self->sections, index);
        if((value->key != NULL && strcmp(value->key, key) == 0) || (value->key == NULL && key == NULL))
            return value;
    }
    return NULL;
}

ini_data_t* ini_file_get_data(ini_file_t* self, char* const section, char* const key)
{
    ini_section_t* section_data = ini_file_get_section(self, section);
    assert(section_data != NULL);
    return ini_section_get_data(section_data, key);
}

ini_data_t* ini_section_get_data(ini_section_t* self, char* const key)
{
    for(uint32_t index = 0; index < self->values.size; index++)
    {
        ini_value_t* const value = dynarray_get(self->values, index);
        if(strcmp(value->key, key) == 0)
            return &value->value;
    }
    return NULL;
}

ini_data_t* ini_data_get_from_array(ini_data_t* self, uint32_t index)
{
    assert(self->type == ARRAY);
    assert(self->data.array.size > index);
    return (ini_data_t*)dynarray_get(self->data.array, index);
}

char* ini_data_get_as_string(ini_data_t* self)
{
    assert(self->type == VALUE);
    return self->data.string;
}

int ini_data_get_as_int(ini_data_t* self)
{
    assert(self->type == VALUE);
    return atoi(self->data.string);
}

bool ini_data_get_as_bool(ini_data_t* self)
{
    assert(self->type == VALUE);
    return strcmp(self->data.string, "true") == 0;
}
