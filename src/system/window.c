#include "system/window.h"

#include <auxum/std.h>

maybe_t app_window_init(app_window_t* self, app_window_init_data_t* const init_data)
{
    maybe_t result;
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        result.ok = false;
        result.error = "Could not initialize SDL3 video subsystem! ";
        return result;
    }

    self->sdl = SDL_CreateWindow(init_data->name, init_data->size_x, init_data->size_y, init_data->flags);
    if(self->sdl == NULL)
    {
        result.ok = false;
        result.error = "Could not create SDL3 window! ";
        return result;        
    }

    self->renderer = SDL_CreateRenderer(self->sdl, NULL);
    if(self->renderer == NULL)
    {
        SDL_DestroyWindow(self->sdl);
        result.ok = false;
        result.error = "Could not create SDL3 renderer! ";
        return result;
    }
    result.ok = true;
    return result;
}

void app_window_free(app_window_t* self)
{
    if(self->renderer != NULL)
        SDL_DestroyRenderer(self->renderer);

    if(self->sdl != NULL)
        SDL_DestroyWindow(self->sdl);
}
