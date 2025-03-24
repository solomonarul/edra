#pragma once
#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <auxum/std.h>
#include <SDL3/SDL.h>

struct app_window {
    SDL_Renderer* renderer;
    SDL_Window* sdl;
};
typedef struct app_window app_window_t;

struct app_window_init_data {
    int size_x;
    int size_y;
    int flags;
    char* name;
};
typedef struct app_window_init_data app_window_init_data_t;
maybe_t app_window_init(app_window_t* self, app_window_init_data_t* const init_data);
void app_window_free(app_window_t* self);

#endif
