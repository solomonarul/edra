#pragma once
#ifndef AUXUM_BITSET_H
#define AUXUM_BITSET_H

struct bitset {
    int size;
    char* data;
};
typedef struct bitset bitset_t;

/*
 *  Initializes a bitset with the specified size.
 */
void bitset_init(bitset_t* const self, int size);

/*
 *  Gets the bit at the specified position.
 */
bool bitset_get(bitset_t* const self, int position);

/*
 *  Sets the bit at the specified position.
 */
void bitset_set(bitset_t* const self, int position);

/*
 *  Resets the bit at the specified position.
 */
void bitset_reset(bitset_t* const self, int position);

/*
 *  XORs the bit at the specified position with a specified value.
 */
void bitset_xor(bitset_t* const self, int position, bool value);

/*
 *  Resizes the bitset to a specified size.
 */
void bitset_resize(bitset_t* const self, int new_size);

/*
 *  Sets the bitset's values to 0.
 */
void bitset_clear(bitset_t* const self);

/*
 *  Frees the bitset's data from memory.
 */
void bitset_free(bitset_t* const self);

#endif
