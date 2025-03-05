#include "drivers/chip8.h"
#include <auxum/std.h>
#include <stdlib.h>

static uint8_t memory_read_b(void* arg, uint16_t address)
{
    cchip8_context_t* const self = arg;
    return self->memory[address];
}

static uint16_t memory_read_w(void* arg, uint16_t address)
{
    cchip8_context_t* const self = arg;
    return (self->memory[address] << 8) | self->memory[address + 1]; 
}

static void memory_write_b(void* arg, uint16_t address, uint8_t value)
{
    cchip8_context_t* const self = arg;
    self->memory[address] = value;
}

static void clear_screen(void* arg)
{
    cchip8_context_t* const self = arg;
    if(self->threaded)
        SDL_LockRWLockForWriting(self->display_lock);

    bitset_clear(&self->display_memory);

    if(self->threaded)
        SDL_UnlockRWLock(self->display_lock);
}

static void resize_screen(void* arg, uint8_t width, uint8_t height)
{
    cchip8_context_t* const self = arg;
    self->state.display_width = width;
    self->state.display_height = height;
    bitset_resize(&self->display_memory, width * height);
}

static void scroll_screen(void* arg, uint8_t amount, chip8_scroll_direction_t direction)
{
    cchip8_context_t* const self = arg;
    UNUSED(amount);
    if(self->threaded)
        SDL_LockRWLockForWriting(self->display_lock);
    switch(direction)
    {
    case CHIP8_SCROLL_DOWN:
        for(uint8_t index = self->state.display_height - 1; index >= amount; index--)
        {
            for(uint8_t pixel = 0; pixel < self->state.display_width; pixel++)
            {
                const int position = pixel + index * self->state.display_width;
                const int otherPosition = pixel + (index - amount) * self->state.display_width;
                bool value = bitset_get(&self->display_memory, otherPosition);
                if(value)
                    bitset_set(&self->display_memory, position);
                else
                    bitset_reset(&self->display_memory, position);
                bitset_reset(&self->display_memory, otherPosition);
            }
        }
        break;
    case CHIP8_SCROLL_LEFT:
        for(uint8_t index = 0; index < self->state.display_height; index++)
        {
            for(uint8_t pixel = 0; pixel < self->state.display_width - amount; pixel++)
            {
                const int position = pixel + index * self->state.display_width;
                const int otherPosition = (pixel + amount) + index * self->state.display_width;
                bool value = bitset_get(&self->display_memory, otherPosition);
                if(value)
                    bitset_set(&self->display_memory, position);
                else
                    bitset_reset(&self->display_memory, position);
                bitset_reset(&self->display_memory, otherPosition);
            }
        }
        break;
    case CHIP8_SCROLL_RIGHT:
        for(uint8_t index = 0; index < self->state.display_height; index++)
        {
            for(uint8_t pixel = self->state.display_width - 1; pixel >= amount; pixel--)
            {
                const int position = pixel + index * self->state.display_width;
                const int otherPosition = (pixel - amount) + index * self->state.display_width;
                bool value = bitset_get(&self->display_memory, otherPosition);
                if(value)
                    bitset_set(&self->display_memory, position);
                else
                    bitset_reset(&self->display_memory, position);
                bitset_reset(&self->display_memory, otherPosition);
            }
        }
        break;
    }
    if(self->threaded)
        SDL_UnlockRWLock(self->display_lock);
}

bool draw_sprite(void* arg, uint16_t address, uint8_t x, uint8_t y, uint8_t n)
{
    cchip8_context_t* const self = arg;
    x %= self->state.display_width;
    y %= self->state.display_height;
    bool collision = false;
    if(self->threaded)
        SDL_LockRWLockForWriting(self->display_lock);
    if(self->state.mode == CHIP8_MODE_SCHIP_MODERN && n == 0)   // Wide drawing.
    {
        n = 0x10;
        for(uint16_t index = address; index < address + n * 2; index += 2)
        {
            const uint16_t word = (self->memory[index] << 8) | self->memory[index + 1];
            for(uint8_t bit = 0; bit < 16; bit++)
            {
                const uint8_t new_x = x + (15 - bit);
                const uint8_t new_y = y + (index - address) / 2;
                if(new_x >= self->state.display_width || new_y >= self->state.display_height) continue;
                const int position = new_x + new_y * self->state.display_width;
                const bool pixel = (word & (1 << bit)) != 0;
                if(pixel == true && bitset_get(&self->display_memory, position) == pixel) collision = true;
                bitset_xor(&self->display_memory, position, pixel);
            }
        }
    }
    else {
        for(uint16_t index = address; index < address + n; index++)
        {
            const uint8_t byte = self->memory[index];
            for(uint8_t bit = 0; bit < 8; bit++)
            {
                const uint8_t new_x = x + (7 - bit);
                const uint8_t new_y = y + (index - address);
                if(new_x >= self->state.display_width || new_y >= self->state.display_height) continue;
                const int position = new_x + new_y * self->state.display_width;
                const bool pixel = (byte & (1 << bit)) != 0;
                if(pixel == true && bitset_get(&self->display_memory, position) == pixel) collision = true;
                bitset_xor(&self->display_memory, position, pixel);
            }
        }
    }
    if(self->threaded)
        SDL_UnlockRWLock(self->display_lock);
    return collision;
}

bool no_key(void* arg, uint8_t index)
{
    UNUSED(arg); UNUSED(index);
    return false;
}

uint8_t get_random(void* arg)
{
    UNUSED(arg);
    return rand();
}

void cchip8_free(cchip8_context_t* self)
{
    bitset_free(&self->display_memory);
    SDL_DestroyRWLock(self->display_lock);
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
    self->state.resize = resize_screen;
    self->state.get_key_status = no_key;
    self->state.scroll = scroll_screen;
    self->state.aux_arg = self;
    
    self->state.display_width = 64;
    self->state.display_height = 32;
    bitset_init(&self->display_memory, 64 * 32);
    self->display_lock = SDL_CreateRWLock();

    // Setup CPU.
    chip8_interpreter_init(&self->cpu.interpreter);
    self->cpu.interpreter.running = true;
    self->cpu.interpreter.state = &self->state;
    self->speed = -1;
    self->threaded = false;

    const uint8_t FONTSET_SIZE = 80;
    static const uint8_t FONTSET[80] = { 
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };  // TODO: make this modifiable from outside?
    memcpy((char*)self->memory, FONTSET, sizeof(uint8_t) * FONTSET_SIZE);
}

#define THREAD_NANOS (SDL_GetPerformanceCounter() * 1.0 / SDL_GetPerformanceFrequency() * SDL_NS_PER_SECOND)

void cchip8_step(cchip8_context_t* self, uint32_t update_rate)
{
    if(!self->cpu.interpreter.running) return;

    long start_time, end_time;
    if(self->speed == (uint32_t)-1)
    {
        start_time = THREAD_NANOS;
        chip8_interpreter_step(&self->cpu.interpreter);
        end_time = THREAD_NANOS;
        chip8_interpreter_update_timers(&self->cpu.interpreter, end_time - start_time + rand() % 100);   // rand() % 100 is a trick cuz THREAD_NANOS is not accurate enough.
    }
    else {
        start_time = 0;
        while(start_time < SDL_NS_PER_SECOND / update_rate)
        {
            chip8_interpreter_step(&self->cpu.interpreter);
            chip8_interpreter_update_timers(&self->cpu.interpreter, SDL_NS_PER_SECOND / self->speed);
            if(!self->cpu.interpreter.running) return;
            start_time += SDL_NS_PER_SECOND / self->speed;
        }
    }
}
