#include "system/window.h"

#include <auxum/std.h>

maybe_t app_window_init(app_window_t* self, app_window_init_data_t* const init_data)
{
    maybe_t result;
#ifdef BUILD_TYPE_VITA
    SDL_SetHint(SDL_HINT_VITA_PVR_OPENGL, "true");
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
#endif

    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        result.ok = false;
        result.error = "Could not initialize SDL3 video subsystem! ";
        return result;
    }

    self->sdl = SDL_CreateWindow(init_data->name, init_data->size_x, init_data->size_y, SDL_WINDOW_OPENGL | init_data->flags);
    if(self->sdl == NULL)
    {
        result.ok = false;
        result.error = "Could not create SDL3 window! ";
        return result;        
    }

    self->context = SDL_GL_CreateContext(self->sdl);
    if(self->context == NULL)
    {
        SDL_DestroyWindow(self->sdl);
        result.ok = false;
        result.error = "Could not create SDL3 GL context! ";
        return result;
    }
    SDL_GL_MakeCurrent(self->sdl, self->context);
    result.ok = true;
    return result;
}

void app_window_free(app_window_t* self)
{
    if(self->context != NULL)
        SDL_GL_DestroyContext(self->context);

    if(self->sdl != NULL)
        SDL_DestroyWindow(self->sdl);
}
