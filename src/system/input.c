#include "system/input.h"

#include <stdlib.h>

void app_input_state_init(app_input_state_t* self)
{
    self->key_state_now = (bool*)SDL_GetKeyboardState(&self->key_count);
    self->key_state_last = calloc(self->key_count, sizeof(bool));
}

void app_input_state_update(app_input_state_t* self, long dt)
{
    UNUSED(dt);
    SDL_memcpy(self->key_state_last, self->key_state_now, sizeof(bool) * self->key_count);
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

void app_input_state_free(app_input_state_t* self)
{
    free(self->key_state_last);
}
