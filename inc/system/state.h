#pragma once
#ifndef APP_STATE_H
#define APP_STATE_H

#include <stddef.h>
#include "window.h"

typedef struct app_state {
    void (*update)(void* data, long dt);
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
size_t app_state_get_count(void);
void app_update(long dt);
void app_render(app_window_t* window);
void app_state_free(void);

#endif
