#pragma once
#ifndef APP_INPUT_H
#define APP_INPUT_H

#include <SDL3/SDL.h>
#include <auxum/std.h>

typedef struct app_input_state {
    int key_count;
    bool* key_state_last;
    bool* key_state_now;
} app_input_state_t;

void app_input_state_init(app_input_state_t* self);
void app_input_state_update(app_input_state_t* self, long dt);
bool app_input_state_key_pressed(app_input_state_t* self, SDL_Scancode key);
bool app_input_state_key_released(app_input_state_t* self, SDL_Scancode key);
bool app_input_state_key_down(app_input_state_t* self, SDL_Scancode key);
void app_input_state_free(app_input_state_t* self);

#endif
