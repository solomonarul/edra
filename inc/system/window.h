#pragma once
#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <auxum/std.h>
#include <auxum/data/dynarray.h>
#include "input.h"

typedef struct app_window {
    SDL_GPUDevice* gpu;
    SDL_Window* sdl;
    app_input_state_t input;
    int size_x, size_y;
} app_window_t;

typedef struct app_window_init_data {
    int size_x;
    int size_y;
    int flags;
    char* name;
} app_window_init_data_t;
maybe_t app_window_init(app_window_t* self, app_window_init_data_t* const init_data);
void app_window_enable_vsync(app_window_t* self, bool status);
void app_window_on_resize(app_window_t* self, int size_x, int size_y);
void app_window_free(app_window_t* self);

void app_show_error(char* const error);
void app_show_sdl_error(char* const error, SDL_Window* parent);

#endif
