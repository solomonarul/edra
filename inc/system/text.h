#pragma once
#ifndef APP_TEXT_H
#define APP_TEXT_H

#include "auxum/std.h"
#include <SDL3_ttf/SDL_ttf.h>

typedef struct app_font {
    int size;
    TTF_Font* data;
} app_font_t;

maybe_t app_font_init_from_path(app_font_t* self, char* const path, float size);
void app_font_free(app_font_t* self);

#endif
