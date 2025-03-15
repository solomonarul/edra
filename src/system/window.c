#include "system/window.h"

#include <auxum/std.h>

maybe_t app_window_init(app_window_t* self, app_window_init_data_t* const init_data)
{
    maybe_t result;
    self->render_type = init_data->render_type;
    int args = 0;
    if(init_data->render_type == APP_RENDER_TYPE_GL)
    {
        args = SDL_WINDOW_OPENGL;
    #ifdef BUILD_TYPE_VITA
        SDL_SetHint(SDL_HINT_VITA_PVR_OPENGL, "true");
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    #endif
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    }

    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        result.ok = false;
        result.error = "Could not initialize SDL3 video subsystem! ";
        return result;
    }

#ifdef BUILD_TYPE_VITA
    self->window = SDL_CreateWindow(NULL, 960, 544, args | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS | init_data->flags);
#else
    self->sdl = SDL_CreateWindow(init_data->name, init_data->size_x, init_data->size_y, args | init_data->flags);
#endif
    if(self->sdl == NULL)
    {
        result.ok = false;
        result.error = "Could not create SDL3 window! ";
        return result;        
    }

    switch(init_data->render_type)
    {
    case APP_RENDER_TYPE_GL:
        self->render_data.context = SDL_GL_CreateContext(self->sdl);
        if(self->render_data.context == NULL)
        {
            SDL_DestroyWindow(self->sdl);
            result.ok = false;
            result.error = "Could not create SDL3 GL context! ";
            return result;
        }
        SDL_GL_MakeCurrent(self->sdl, self->render_data.context);    
        break;
    
    case APP_RENDER_TYPE_SDL_RENDERER:
        self->render_data.renderer = SDL_CreateRenderer(self->sdl, NULL);
        if(self->render_data.renderer == NULL)
        {
            SDL_DestroyWindow(self->sdl);
            result.ok = false;
            result.error = "Could not create SDL3 renderer! ";
            return result;
        }
        break;
    
    default:
        result.ok = false;
        result.error = "Render type not supported.";
        return result;
    }

    result.ok = true;
    return result;
}

void app_window_free(app_window_t* self)
{
    switch(self->render_type)
    {
    case APP_RENDER_TYPE_GL:
        if(self->render_data.context != NULL)
            SDL_GL_DestroyContext(self->render_data.context);
        break;

    case APP_RENDER_TYPE_SDL_RENDERER:
        if(self->render_data.renderer != NULL)
            SDL_DestroyRenderer(self->render_data.renderer);
        break;

    case APP_RENDER_TYPE_NONE:
        break;
    }

    if(self->sdl != NULL)
        SDL_DestroyWindow(self->sdl);
}
