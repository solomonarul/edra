#include "system/input.h"
#include "auxum/data/dynarray.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <stdlib.h>
#include <stdio.h>

bool app_input_state_gamepad_button_pressed(app_input_gamepad_state_t* self, SDL_GamepadButton button)
{
    if(button >= SDL_GAMEPAD_BUTTON_COUNT)
        return false;
    return self->buttons[button] && !self->last_buttons[button];
}

bool app_input_state_gamepad_button_released(app_input_gamepad_state_t* self, SDL_GamepadButton button)
{
    if(button >= SDL_GAMEPAD_BUTTON_COUNT)
        return false;
    return !self->buttons[button] && self->last_buttons[button];
}

bool app_input_state_gamepad_button_down(app_input_gamepad_state_t* self, SDL_GamepadButton button)
{
    if(button >= SDL_GAMEPAD_BUTTON_COUNT)
        return false;
    return self->buttons[button];
}

void app_input_state_init(app_input_state_t* self)
{
    self->key_state_now = (bool*)SDL_GetKeyboardState(&self->key_count);
    self->key_state_last = calloc(self->key_count, sizeof(bool));
    dynarray_init(&self->gamepads, sizeof(app_input_gamepad_state_t), 0);
}

void app_input_state_update(app_input_state_t* self, long dt)
{
    UNUSED(dt);
    SDL_memcpy(self->key_state_last, self->key_state_now, sizeof(bool) * self->key_count);
    for(size_t index = 0; index < self->gamepads.size; index++)
    {
        app_input_gamepad_state_t* const gamepad = dynarray_get(self->gamepads, index);
        SDL_memcpy(gamepad->last_buttons, gamepad->buttons, sizeof(bool) * SDL_GAMEPAD_BUTTON_COUNT);
        for(size_t button = 0; button < SDL_GAMEPAD_BUTTON_COUNT; button++)
            gamepad->buttons[button] = SDL_GetGamepadButton(gamepad->sdl, button);
    }
}

bool app_input_state_key_pressed(app_input_state_t* self, SDL_Scancode key)
{
    return self->key_state_now[key] && !self->key_state_last[key];
}

bool app_input_state_key_released(app_input_state_t* self, SDL_Scancode key)
{
    return !self->key_state_now[key] && self->key_state_last[key];
}

bool app_input_state_key_down(app_input_state_t* self, SDL_Scancode key)
{
    return self->key_state_now[key];
}

void app_input_state_on_event(app_input_state_t* self, SDL_Event* event)
{
    switch(event->type)
    {
    case SDL_EVENT_GAMEPAD_ADDED:
    {
        SDL_JoystickID id = event->gdevice.which;
        SDL_Gamepad* gamepad = SDL_OpenGamepad(id);
        app_input_gamepad_state_t state = {0};
        state.sdl = gamepad;
        dynarray_push_back(&self->gamepads, &state);
        printf("[INFO]: Gamepad with id %d (%s) has been connected!\n", id, SDL_GetGamepadName(gamepad));
        break;
    }

    case SDL_EVENT_GAMEPAD_REMOVED:
    {
        SDL_JoystickID removed_id = event->gdevice.which;
        for(size_t index = 0; index < self->gamepads.size; index++)
        {
            app_input_gamepad_state_t* const gamepad = dynarray_get(self->gamepads, index);
            if(SDL_GetGamepadID(gamepad->sdl) == removed_id)
            {
                printf("[INFO]: Gamepad with id %d has been removed!\n", removed_id);
                SDL_CloseGamepad(gamepad->sdl);
                dynarray_remove(&self->gamepads, index);
                break;
            }
        }
        break;
    }
    }
}

app_input_gamepad_state_t* app_input_state_get_gamepad(app_input_state_t* self, size_t index)
{
    if(self->gamepads.size <= index)
        return NULL;
    return dynarray_get(self->gamepads, index);
}

void app_input_state_free(app_input_state_t* self)
{
    free(self->key_state_last);
}
