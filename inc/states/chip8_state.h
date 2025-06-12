#pragma once

#ifndef CCHIP8_STATE_H
#define CCHIP8_STATE_H

#include "../system/state.h"
#include "../drivers/chip8.h"
#include "states/chip8_pause_state.h"
#include <auxum/std.h>

typedef struct chip8_app_state {
    app_state_t internal;
    cchip8_context_t emulator;
    chip8_pause_app_state_t pause_state;
    SDL_Thread* thread;
} chip8_app_state_t;

void chip8_app_state_init(chip8_app_state_t* self);
maybe_t chip8_app_state_load_rom(chip8_app_state_t* self, chip8_run_mode_t run_mode, FILE* rom, bool threaded, bool close_file_on_read);

#endif
