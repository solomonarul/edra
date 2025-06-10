#include "system/state.h"

#include <auxum/data/dynarray.h>

static dynarray_t states;

void app_state_init(void)
{
    dynarray_init(&states, sizeof(app_state_t), 0);
}

void app_state_push(app_state_t state)
{
    dynarray_push_back(&states, &state);
}

app_state_t app_state_pop(void)
{
    app_state_t state = *(app_state_t*)dynarray_get(states, states.size - 1);
    dynarray_pop_back(&states);
    return state;
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

void app_update(long dt)
{
    for(size_t index = 0; index < states.size; index++)
    {
        app_state_t* const state = dynarray_get(states, index);
        if(state->update != NULL)
            state->update(state->userdata, dt);
    }
}

void app_render(app_window_t* window)
{
    for(size_t index = 0; index < states.size; index++)
    {
        app_state_t* const state = dynarray_get(states, index);
        if(state->render != NULL)
            state->render(state->userdata, window);
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
