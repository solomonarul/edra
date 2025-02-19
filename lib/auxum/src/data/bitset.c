#include "data/bitset.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void bitset_init(bitset_t* const self, int size)
{
    self->size = size;
    self->data = calloc((size / sizeof(char)) + 1, sizeof(char));
}

bool bitset_get(bitset_t* const self, int position)
{
    assert(position < self->size);
    return (self->data[position / 8] & (1 << (position % 8))) != 0;
}

void bitset_set(bitset_t* const self, int position)
{
    assert(position < self->size);
    self->data[position / 8] |= 1 << (position % 8);
}

void bitset_reset(bitset_t* const self, int position)
{
    assert(position < self->size);
    self->data[position / 8] &= ~(1 << (position % 8));
}

void bitset_xor(bitset_t* const self, int position, bool value)
{
    assert(position < self->size);
    self->data[position / 8] ^= value ? (1 << (position % 8)) : 0;
}

void bitset_resize(bitset_t* const self, int new_size)
{
    assert(new_size != 0);
    self->data = realloc(self->data, (new_size / sizeof(char)) + 1);
}

void bitset_clear(bitset_t* self)
{
    memset(self->data, 0, ((self->size / sizeof(char)) + 1) * sizeof(char));
}

void bitset_free(bitset_t* const self)
{
    free(self->data);
    self->data = NULL;
}
