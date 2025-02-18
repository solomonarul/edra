#pragma once
#ifndef CCHIP8_DRIVER_H
#define CCHIP8_DRIVER_H

#include <auxum/bitset.h>
#include <auxum/ini.h>
#include <cchip8/state.h>
#include <cchip8/cpu/interpreter.h>
#include <SDL3/SDL.h>

struct cchip8_context {
    chip8_state_t state;
    bitset_t display_memory;
    SDL_RWLock* display_lock;
    uint32_t speed;
    union {
        chip8_interpreter_t interpreter;
    } cpu;
    uint8_t memory[0x10000];
};
typedef struct cchip8_context cchip8_context_t;

void cchip8_init(cchip8_context_t* self);
void cchip8_step(cchip8_context_t* self, uint32_t update_rate);
void cchip8_run_sdl(cchip8_context_t* self, ini_file_t* config, bool threaded);
void cchip8_free(cchip8_context_t* self);

#endif
