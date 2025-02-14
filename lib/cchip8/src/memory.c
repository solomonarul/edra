#include "memory.h"

#include <assert.h>

uint8_t* base_address = NULL;

void chip8_memory_set_base(uint8_t* location)
{
    base_address = location;
}

uint8_t chip8_memory_read_b(uint16_t address)
{
    assert(base_address != NULL);
    return base_address[address];
}

uint16_t chip8_memory_read_w(uint16_t address)
{
    assert(base_address != NULL);
    return (base_address[address] << 8) | base_address[address + 1]; 
}

void chip8_memory_write_b(uint16_t address, uint8_t value)
{
    assert(base_address != NULL);
    base_address[address] = value;
}
