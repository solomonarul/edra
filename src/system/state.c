#include "system/state.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_timer.h>
#include <auxum/data/dynarray.h>
#include "system/input.h"
#include "system/window.h"

static dynarray_t states;

void app_state_init(void)
{
    dynarray_init(&states, sizeof(app_state_t), 0);
}

void app_state_push(app_state_t state)
{
    dynarray_push_back(&states, &state);
}

#include <stdio.h>

app_state_t app_state_pop(void)
{
    app_state_t state = *(app_state_t*)dynarray_get(states, states.size - 1);
    dynarray_pop_back(&states);
    return state;
}

app_state_t* app_state_top(void)
{
    return (app_state_t*)dynarray_get(states, states.size - 1);
}

app_state_t app_state_remove(size_t index)
{
    app_state_t state = *(app_state_t*)dynarray_get(states, index);
    dynarray_remove(&states, index);
    return state;    
}

size_t app_state_get_count(void)
{
    return states.size;
}

void app_update(app_window_t* window, long dt)
{
    for(size_t index = 0; index < states.size; index++)
    {
        app_state_t* const state = dynarray_get(states, index);
        if(state->update != NULL)
            state->update(state->userdata, window, dt);
    }
}

void app_render(app_window_t* window, SDL_GPUCommandBuffer* buffer, SDL_GPUTexture* swapchainTexture)
{
    for(size_t index = 0; index < states.size; index++)
    {
        app_state_t* const state = dynarray_get(states, index);
        if(state->render != NULL)
            state->render(state->userdata, window, buffer, swapchainTexture);
    }
}

void app_state_free(void)
{
    for(size_t index = 0; index < states.size; index++)
    {
        app_state_t* const state = dynarray_get(states, index);
        if(state->free != NULL)
            state->free(state->userdata);
    }
    dynarray_free(states);
}

void app_run_main_loop(app_window_t* self)
{
    SDL_Event event;
    bool app_running = true;
    long start_time, end_time;
    start_time = end_time = THREAD_NANOS;
    while(app_running)
    {
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_EVENT_GAMEPAD_ADDED:
            case SDL_EVENT_GAMEPAD_REMOVED:
                app_input_state_on_event(&self->input, &event);
                break;

            case SDL_EVENT_QUIT:
                app_running = false;
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                app_window_on_resize(self, event.window.data1, event.window.data2);
                break;
            }
        }
        end_time = THREAD_NANOS;
        app_update(self, end_time - start_time);
        app_input_state_update(&self->input, end_time - start_time);
        start_time = end_time;

        SDL_GPUCommandBuffer* buffer = SDL_AcquireGPUCommandBuffer(self->gpu);
        if(!buffer)
        {
            // TODO: error out.
            return;
        }

        SDL_GPUTexture* swapchainTexture;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(buffer, self->sdl, &swapchainTexture, NULL, NULL))
        {
            // TODO: error out.
            return;
        }

        app_render(self, buffer, swapchainTexture);
        
        SDL_SubmitGPUCommandBuffer(buffer);
    }
}
