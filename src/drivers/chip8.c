#include "drivers/chip8.h"
#include <auxum/unused.h>
#include <auxum/thread.h>
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
    self->state.get_random = get_random;
    self->state.aux_arg = self;
    bitset_init(&self->display_memory, 64 * 32);

    // Setup CPU.
    chip8_interpreter_init(&self->cpu.interpreter);
    self->cpu.interpreter.running = true;
    self->cpu.interpreter.state = &self->state;
    self->speed = -1;
}

void cchip8_run(cchip8_context_t* self)
{
    long start_time, end_time;
    long timer;
    while(true)
    {
        if(self->speed == (uint32_t)-1)
        {
            start_time = thread_get_nanos();
            chip8_interpreter_step(&self->cpu.interpreter);
            end_time = thread_get_nanos();
            chip8_interpreter_update_timers(&self->cpu.interpreter, end_time - start_time + rand() % 100);   // rand() % 100 is a trick cuz the timer is not accurate enough
            if(!self->cpu.interpreter.running) break;
        }
        else {
            timer = 0;
            while(timer < 1e9 / 60)
            {
                chip8_interpreter_step(&self->cpu.interpreter);
                chip8_interpreter_update_timers(&self->cpu.interpreter, 1e9 / self->speed);
                if(!self->cpu.interpreter.running) break;
                timer += 1e9 / self->speed;
            }
            if(!self->cpu.interpreter.running) break;
            thread_spin_sleep(1e9 / 60, -1);
        }
    }
    fprintf(stderr, "[INFO]: Emulator has stopped.\n");
}

void cchip8_free(cchip8_context_t* self)
{
    bitset_free(&self->display_memory);
}
