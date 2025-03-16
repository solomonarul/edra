#pragma once
#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <auxum/std.h>
#include <SDL3/SDL.h>
#ifdef BUILD_TYPE_VITA
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

struct app_window {
    SDL_GLContext context;
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
