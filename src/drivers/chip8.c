#include "drivers/chip8.h"
#include <auxum/unused.h>
#include <SDL3/SDL.h>
#include <assert.h>
#include <threads.h>

uint8_t memory_read_b(void* arg, uint16_t address)
{
    cchip8_context_t* const self = arg;
    return self->memory[address];
}

uint16_t memory_read_w(void* arg, uint16_t address)
{
    cchip8_context_t* const self = arg;
    return (self->memory[address] << 8) | self->memory[address + 1]; 
}

void memory_write_b(void* arg, uint16_t address, uint8_t value)
{
    cchip8_context_t* const self = arg;
    self->memory[address] = value;
}

void clear_screen(void* arg)
{
    cchip8_context_t* const self = arg;
    bitset_clear(&self->display_memory);
}

bool draw_sprite(void* arg, uint16_t address, uint8_t x, uint8_t y, uint8_t n)
{
    cchip8_context_t* const self = arg;
    x %= 64;
    y %= 32;
    bool collision = false;
    for(uint16_t index = address; index < address + n; index++)
    {
        const uint8_t byte = self->memory[index];
        for(uint8_t bit = 0; bit < 8; bit++)
        {
            const uint8_t new_x = x + (7 - bit);
            const uint8_t new_y = y + (index - address);
            if(new_x > 63 || new_y > 31) continue;
            const int position = new_x + new_y * 64;
            const bool pixel = (byte & (1 << bit)) != 0;
            if(pixel == true && bitset_get(&self->display_memory, position) == pixel) collision = true;
            bitset_xor(&self->display_memory, position, pixel);
        }
    }
    return collision;
}

bool get_key_status(void* arg, uint8_t key)
{
    // TODO: maybe get this SDL out too?
    UNUSED(arg);
    static SDL_Scancode keys[0x10] = {
        SDL_SCANCODE_X,
        SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
        SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E,
        SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D,
        SDL_SCANCODE_Z,
        SDL_SCANCODE_C, SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F,
        SDL_SCANCODE_V,
    };
    static int keyboard_state_length;
    static const bool* keyboard_state;
    keyboard_state = SDL_GetKeyboardState(&keyboard_state_length);
    assert(key < 0x10);
    return keyboard_state[keys[key]];
}

uint8_t get_random(void* arg)
{
    UNUSED(arg);
    return rand();
}

void cchip8_init(cchip8_context_t* self)
{
    // Setup state and callbacks.
    chip8_state_init(&self->state);
    self->state.read_b = memory_read_b;
    self->state.read_w = memory_read_w;
    self->state.write_b = memory_write_b;
    self->state.draw_sprite = draw_sprite;
    self->state.clear_screen = clear_screen;
    self->state.get_key_status = get_key_status;
    self->state.get_random = get_random;
    self->state.aux_arg = self;
    bitset_init(&self->display_memory, 64 * 32);

    // Setup CPU. TODO: preferably on the stack
    self->cpu = malloc(sizeof(chip8_interpreter_t));
    chip8_interpreter_init(self->cpu);
    self->cpu->running = true;
    self->cpu->state = &self->state;
}

void cchip8_run(cchip8_context_t* self)
{
    const int speed = 1000;
    while(true)
    {
        chip8_interpreter_step(self->cpu);
        chip8_interpreter_update_timers(self->cpu, 1.0 / speed);
        if(!self->cpu->running) break;

        // TODO: maybe not everything supports C23, generalize later.
        if(speed > 0)
            thrd_sleep(&(struct timespec){.tv_nsec=1000000000 / speed}, NULL);    // TODO: not really accurate let's implement our own spinlocker later.
    }
    fprintf(stderr, "[INFO]: Emulator has stopped.");
}

void cchip8_free(cchip8_context_t* self)
{
    bitset_free(&self->display_memory);
    free(self->cpu);
}
