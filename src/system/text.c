#include "system/text.h"

static bool ttf_was_init = false;

maybe_t app_font_init_from_path(app_font_t* self, char* const path, float size)
{
    maybe_t result;
    if(!ttf_was_init)
    {
        if(!TTF_Init())
        {
            result.ok = false;
            result.error = "SDL_TTF could not be initialized.";
            return result;
        }
        ttf_was_init = true;
    }

    self->data = TTF_OpenFont(path, size);
    if(!self->data)
    {
        result.ok = false;
        result.error = "Couldn't open font.";
        return result;
    }

    result.ok = true;
    return result;
}

void app_font_free(app_font_t* self)
{
    if(self->data)
        TTF_CloseFont(self->data);
}
