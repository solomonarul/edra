#pragma once
#ifndef APP_STATE_H
#define APP_STATE_H

#include <stddef.h>
#include "window.h"

typedef struct app_state {
    void (*update)(void* data, app_window_t* window, long dt);
    void (*render)(void* data, app_window_t* window);
    void (*pause)(void* data);
    void (*unpause)(void* data);
    void (*free)(void*);
    void* userdata;
} app_state_t;

void app_state_init(void);
void app_state_push(app_state_t state);
app_state_t app_state_pop(void);
app_state_t app_state_remove(size_t index);
app_state_t* app_state_top(void);
size_t app_state_get_count(void);
void app_update(app_window_t* self, long dt);
void app_render(app_window_t* window);
void app_state_free(void);

#define THREAD_NANOS (SDL_GetPerformanceCounter() * 1.0 / SDL_GetPerformanceFrequency() * SDL_NS_PER_SECOND)

void app_run_main_loop(app_window_t* self);

#endif
