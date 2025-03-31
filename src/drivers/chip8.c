#include "drivers/chip8.h"
#include <auxum/std.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

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

    // TODO: maybe this shouldn't be available unless in SCHIP mode.
    static const uint8_t HIRES_FONTSET_SIZE = 80;
    static const uint16_t HIRES_FONTSET[80] = {
        0xC67C, 0xDECE, 0xF6D6, 0xC6E6, 0x007C, // 0
        0x3010, 0x30F0, 0x3030, 0x3030, 0x00FC, // 1
        0xCC78, 0x0CCC, 0x3018, 0xCC60, 0x00FC, // 2
        0xCC78, 0x0C0C, 0x0C38, 0xCC0C, 0x0078, // 3
        0x1C0C, 0x6C3C, 0xFECC, 0x0C0C, 0x001E, // 4
        0xC0FC, 0xC0C0, 0x0CF8, 0xCC0C, 0x0078, // 5
        0x6038, 0xC0C0, 0xCCF8, 0xCCCC, 0x0078, // 6
        0xC6FE, 0x06C6, 0x180C, 0x3030, 0x0030, // 7
        0xCC78, 0xECCC, 0xDC78, 0xCCCC, 0x0078, // 8
        0xC67C, 0xC6C6, 0x0C7E, 0x3018, 0x0070, // 9
        0x7830, 0xCCCC, 0xFCCC, 0xCCCC, 0x00CC, // A
        0x66FC, 0x6666, 0x667C, 0x6666, 0x00FC, // B
        0x663C, 0xC0C6, 0xC0C0, 0x66C6, 0x003C, // C
        0x6CF8, 0x6666, 0x6666, 0x6C66, 0x00F8, // D
        0x62FE, 0x6460, 0x647C, 0x6260, 0x00FE, // E
        0x66FE, 0x6462, 0x647C, 0x6060, 0x00F0 // F
    };
    memcpy((char*)(self->memory + self->state.lowres_font_address), FONTSET, sizeof(uint8_t) * FONTSET_SIZE);
    memcpy((char*)(self->memory + self->state.hires_font_address), HIRES_FONTSET, sizeof(uint8_t) * HIRES_FONTSET_SIZE);
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

void cchip8_draw_sdl(cchip8_context_t* self, SDL_Renderer* renderer)
{
    SDL_SetRenderLogicalPresentation(renderer, self->state.display_width, self->state.display_height, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    
    SDL_LockRWLockForReading(self->display_lock);
    for(uint8_t x = 0; x < self->state.display_width; x++)
        for(uint8_t y = 0; y < self->state.display_height; y++)
            if(bitset_get(&self->display_memory, x + y * self->state.display_width))
                SDL_RenderPoint(renderer, x, y);
    SDL_UnlockRWLock(self->display_lock);
}

bool cchip8_get_sdl_key_status(void* arg, uint8_t key)
{
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
    return keyboard_state[keys[key]];
}

int cchip8_cpu_thread_function(void* args)
{
    cchip8_context_t* const self = args;
    while(true)
    {
        if(self->speed != (uint32_t)-1)
        {
            cchip8_step(self, 75);
            SDL_DelayNS(SDL_NS_PER_SECOND / 75);
        }
        else
            cchip8_step(self, SDL_NS_PER_SECOND);

        if(!self->cpu.interpreter.running) break;
    }
    fprintf(stdout, "[CHP8] Emulator has stopped.\n");
    return 0;
}
