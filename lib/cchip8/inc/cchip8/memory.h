#pragma once
#ifndef CHIP8_MEMORY_H
#define CHIP8_MEMORY_H

#include <stdint.h>

// Default functions to be used when reading / writing memory.
void chip8_memory_set_base(uint8_t* location);
uint8_t chip8_memory_read_b(uint16_t address);
uint16_t chip8_memory_read_w(uint16_t address);
void chip8_memory_write_b(uint16_t address, uint8_t value);

#endif
