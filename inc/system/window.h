#pragma once
#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <auxum/std.h>
#include <SDL3/SDL.h>

struct app_window {
    SDL_Renderer* renderer;
    SDL_Window* sdl;
    int size_x, size_y;
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
void app_window_on_resize(app_window_t* self, int size_x, int size_y);
void app_window_free(app_window_t* self);

void app_show_error(char* const error);
void app_show_sdl_error(char* const error, SDL_Window* parent);

#endif
