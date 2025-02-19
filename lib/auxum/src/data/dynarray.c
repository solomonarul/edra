#include "data/dynarray.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void dynarray_init(dynarray_t* self, uint32_t element_size, uint32_t array_size)
{
    assert(element_size != 0);
    self->element_size = element_size;
    self->size = array_size;
    self->capacity = 1;
    self->free_callback = NULL;
    while(self->capacity <= self->size)
        self->capacity *= 2;
    self->data = calloc(self->capacity, element_size);
}

void* dynarray_push_back(dynarray_t* self, void* element)
{
    if(self->size == self->capacity)
    {
        self->capacity *= 2;
        self->data = realloc(self->data, self->capacity * self->element_size);
        assert(self->data != NULL);
    }
    char* address = (char*)self->data + (self->size * self->element_size);
    memcpy(address, element, self->element_size);
    self->size++;
    return address;
}

void dynarray_pop_back(dynarray_t *self)
{
    if(self->size <= 0)
        return;
    self->size--;
    if(self->size <= self->capacity / 2 && self->capacity > 1)
    {
        self->capacity /= 2;
        self->data = realloc(self->data, self->capacity * self->element_size);
        assert(self->data != NULL);
    }
}

void dynarray_remove(dynarray_t* self, uint32_t index)
{
    if(index >= self->size)
        return;
    if(self->free_callback != NULL)
        self->free_callback((char*)self->data + (index * self->element_size));
    memmove((char*)self->data + (index * self->element_size), (char*)self->data + ((index + 1) * self->element_size), (self->size - index - 1) * self->element_size);
    self->size--;
    if(self->size < self->capacity / 4)
    {
        self->capacity /= 4;
        self->data = realloc(self->data, self->capacity * self->element_size);
        assert(self->data != NULL);
    }
}

void* dynarray_get(dynarray_t self, uint32_t index)
{
    if(index >= self.size)
        return NULL;
    return (char*)self.data + (index * self.element_size);
}

void dynarray_free(dynarray_t self)
{
    if(self.free_callback != NULL)
        for(uint32_t index = 0; index < self.size; index++)
            self.free_callback((char*)self.data + (index * self.element_size));
    free(self.data);
}
