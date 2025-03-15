#pragma once
#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <auxum/std.h>
#include <SDL3/SDL.h>
#include <GL/gl.h>

enum app_window_render_type {
    APP_RENDER_TYPE_NONE = 0,
    APP_RENDER_TYPE_SDL_RENDERER,
    APP_RENDER_TYPE_GL
};
typedef enum app_window_render_type app_window_render_type_t;

struct app_window {
    app_window_render_type_t render_type;
    union {
        SDL_Renderer* renderer;
        SDL_GLContext context;
    } render_data;
    SDL_Window* sdl;
};
typedef struct app_window app_window_t;

struct app_window_init_data {
    int size_x;
    int size_y;
    int flags;
    char* name;
    app_window_render_type_t render_type;
};
typedef struct app_window_init_data app_window_init_data_t;
maybe_t app_window_init(app_window_t* self, app_window_init_data_t* const init_data);
void app_window_free(app_window_t* self);

#endif
