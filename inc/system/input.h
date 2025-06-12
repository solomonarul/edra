#pragma once
#ifndef APP_INPUT_H
#define APP_INPUT_H

#include <SDL3/SDL.h>
#include <auxum/std.h>
#include <auxum/data/dynarray.h>

typedef struct app_input_gamepad_state {
    SDL_Gamepad* sdl;
    bool buttons[SDL_GAMEPAD_BUTTON_COUNT], last_buttons[SDL_GAMEPAD_BUTTON_COUNT];
} app_input_gamepad_state_t;

bool app_input_state_gamepad_button_pressed(app_input_gamepad_state_t* self, SDL_GamepadButton button);
bool app_input_state_gamepad_button_released(app_input_gamepad_state_t* self, SDL_GamepadButton button);
bool app_input_state_gamepad_button_down(app_input_gamepad_state_t* self, SDL_GamepadButton button);

typedef struct app_input_state {
    int key_count;
    bool* key_state_last;
    bool* key_state_now;
    dynarray_t gamepads;
} app_input_state_t;

void app_input_state_init(app_input_state_t* self);
void app_input_state_update(app_input_state_t* self, long dt);
bool app_input_state_key_pressed(app_input_state_t* self, SDL_Scancode key);
bool app_input_state_key_released(app_input_state_t* self, SDL_Scancode key);
bool app_input_state_key_down(app_input_state_t* self, SDL_Scancode key);
void app_input_state_on_event(app_input_state_t* self, SDL_Event* event);
app_input_gamepad_state_t* app_input_state_get_gamepad(app_input_state_t* self, size_t index);
void app_input_state_free(app_input_state_t* self);

#endif
