#pragma once
#ifndef CCHIP8_DRIVER_H
#define CCHIP8_DRIVER_H

#include "../system/window.h"
#include <auxum/data.h>
#include <cchip8/state.h>
#include <cchip8/cpu/interpreter.h>
#include <SDL3/SDL.h>

typedef struct cchip8_context {
    bool threaded;
    chip8_state_t state;
    bitset_t display_memory;
    SDL_RWLock* display_lock;
    uint32_t speed;
    union {
        chip8_interpreter_t interpreter;
    } cpu;
    uint8_t memory[0x10000];
    app_input_state_t* input;
} cchip8_context_t;

void cchip8_init(cchip8_context_t* self);
void cchip8_free(cchip8_context_t* self);
void cchip8_step(cchip8_context_t* self, uint32_t update_rate);
void cchip8_draw_sdl(cchip8_context_t* self, SDL_Renderer* renderer);
bool cchip8_get_sdl_key_status(void* arg, uint8_t key);
void cchip8_load_default_font(cchip8_context_t* self);
void cchip8_load_default_font_hires(cchip8_context_t* self);
void cchip8_load_default_rom(cchip8_context_t* self);
int cchip8_cpu_thread_function(void* args);

#endif
