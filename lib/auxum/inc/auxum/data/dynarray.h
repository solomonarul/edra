#pragma once
#ifndef AUXUM_DYNARRAY_H
#define AUXUM_DYNARRAY_H

#include <stdint.h>

typedef void(*dynarray_free_callback_f)(void*);

typedef struct {
    void* data;
    uint32_t size, capacity, element_size;
    dynarray_free_callback_f free_callback;
} dynarray_t;

void dynarray_init(dynarray_t* self, uint32_t element_size, uint32_t array_size);
void* dynarray_push_back(dynarray_t* self, void* element);
void dynarray_pop_back(dynarray_t* self);
void* dynarray_get(dynarray_t self, uint32_t index);
void dynarray_remove(dynarray_t* self, uint32_t index);
void dynarray_free(dynarray_t self);

#endif
