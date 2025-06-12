#include "states/chip8_pause_state.h"
#include "clay_renderer_SDL3.h"

#include <stdio.h>

static void chip8_pause_state_app_update(void* self_ref, app_window_t* window, long dt)
{
    chip8_pause_app_state_t* const self = (chip8_pause_app_state_t*)self_ref;
    UNUSED(dt);
    if(!self->update_skipped_first_frame)
    {
        self->update_skipped_first_frame = true;
        return;
    }

    if(app_input_state_key_pressed(&window->input, SDL_SCANCODE_ESCAPE))
    {
        app_state_pop();
        app_state_t* previous = app_state_top();
        previous->unpause(previous->userdata);
    }
}

#ifdef BUILD_TYPE_VITA
    #define PAUSE_MESSAGE "PRESS START TO RESUME."
#else
    #define PAUSE_MESSAGE "PRESS ESCAPE TO RESUME."
#endif

static Clay_RenderCommandArray chip8_pause_state_app_get_render_layout(void)
{
    Clay_BeginLayout();
    CLAY({
        .id = CLAY_ID("OuterContainer"),
        .layout = {
            .childAlignment = {.x = CLAY_ALIGN_X_CENTER},
            .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
            .padding = CLAY_PADDING_ALL(16),
            .childGap = 16
        },
        .backgroundColor = {5,5,5,200}
    }) {
        CLAY({
            .id = CLAY_ID("SideBar"),
            .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0)
                },
                .padding = CLAY_PADDING_ALL(16),
                .childGap = 16
            },
            .backgroundColor = {55,55,55,50}
        }) {
            CLAY({
                .layout = {
                    .childAlignment = {.y = CLAY_ALIGN_Y_CENTER, .x = CLAY_ALIGN_X_CENTER},
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = {
                        .width = CLAY_SIZING_GROW(0),
                        .height = CLAY_SIZING_GROW(0)
                    },
                    .padding = CLAY_PADDING_ALL(16),
                    .childGap = 16
                },
            }) {
                CLAY_TEXT(
                    CLAY_STRING("PAUSED"), 
                    CLAY_TEXT_CONFIG({
                        .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                        .fontSize = 24,
                        .textColor = {255, 255, 255, 255},
                        .fontId = 0
                    })
                );
                CLAY_TEXT(
                    CLAY_STRING(PAUSE_MESSAGE),
                    CLAY_TEXT_CONFIG({
                        .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                        .fontSize = 24,
                        .textColor = {255, 255, 255, 255},
                        .fontId = 1
                    })
                );
            };
        }
    }

    return Clay_EndLayout();
}

#undef PAUSE_MESSAGE

static void chip8_pause_state_app_render(void* self_ref, app_window_t* window)
{
    chip8_pause_app_state_t* const self = (chip8_pause_app_state_t*)self_ref;
    UNUSED(self);
    Clay_RenderCommandArray render_pipeline = chip8_pause_state_app_get_render_layout();
    SDL_Clay_RenderClayCommands(&window->clay_renderer, &render_pipeline);
}

static void chip8_pause_state_app_pause(void* self_ref)
{
    chip8_pause_app_state_t* const self = (chip8_pause_app_state_t*)self_ref;
    if(self->internal.paused)
        return;
}

static void chip8_pause_state_app_unpause(void* self_ref)
{
    chip8_pause_app_state_t* const self = (chip8_pause_app_state_t*)self_ref;
    if(!self->internal.paused)
        return;
}

static void chip8_pause_state_app_free(void* self_ref)
{
    chip8_pause_app_state_t* const self = (chip8_pause_app_state_t*)self_ref;
    UNUSED(self);
}

void chip8_pause_app_state_init(chip8_pause_app_state_t* self)
{
    self->update_skipped_first_frame = false;
    self->internal.userdata = self;
    self->internal.free = chip8_pause_state_app_free;
    self->internal.unpause = chip8_pause_state_app_unpause;
    self->internal.pause = chip8_pause_state_app_pause;
    self->internal.render = chip8_pause_state_app_render;
    self->internal.update = chip8_pause_state_app_update;
}
