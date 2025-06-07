#include "system/window.h"

#include <auxum/std.h>
#include <stdlib.h>
#include <stdio.h>

static bool sdl_video_was_init = false;

maybe_t app_window_init(app_window_t* self, app_window_init_data_t* const init_data)
{
    maybe_t result;
    if(!sdl_video_was_init)
    {
        if(!SDL_InitSubSystem(SDL_INIT_VIDEO))
        {
            result.ok = false;
            result.error = "Could not initialize SDL3 video subsystem! ";
            return result;
        }
        sdl_video_was_init = true; 
    }

    self->sdl = SDL_CreateWindow(init_data->name, init_data->size_x, init_data->size_y, init_data->flags);
    if(self->sdl == NULL)
    {
        result.ok = false;
        result.error = "Could not create SDL3 window! ";
        return result;        
    }
    self->size_x = init_data->size_x;
    self->size_y = init_data->size_y;

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

void app_window_enable_vsync(app_window_t* self, bool status)
{
    SDL_SetRenderVSync(self->renderer, status);
}

void app_window_on_resize(app_window_t* self, int size_x, int size_y)
{
    self->size_x = size_x;
    self->size_y = size_y;
}

void app_window_free(app_window_t* self)
{
    if(self->renderer != NULL)
        SDL_DestroyRenderer(self->renderer);

    if(self->sdl != NULL)
        SDL_DestroyWindow(self->sdl);
}

void app_show_error(char* const error)
{
    fprintf(stderr, "[EROR] %s\n", error);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", error, NULL);
    exit(-1);
}

void app_show_sdl_error(char* const error, SDL_Window* parent)
{
    if(SDL_GetError() == NULL)
        app_show_error(error);

    fprintf(stderr, "[EROR] %s(SDL Error: %s)\n", error, SDL_GetError());
    int first_length = strlen(error);
    int second_length = strlen(SDL_GetError());
    char result[first_length + second_length + 1];
    string_nconcat(result, error, first_length, (char* const)SDL_GetError(), second_length);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", result, parent);
    exit(-1);
}
