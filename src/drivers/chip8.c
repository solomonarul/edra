#include "drivers/chip8.h"
#include <auxum/unused.h>
#include <auxum/strings.h>
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
    SDL_LockRWLockForWriting(self->display_lock);
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
    SDL_UnlockRWLock(self->display_lock);
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
    self->display_lock = SDL_CreateRWLock();

    // Setup CPU.
    chip8_interpreter_init(&self->cpu.interpreter);
    self->cpu.interpreter.running = true;
    self->cpu.interpreter.state = &self->state;
    self->speed = -1;

    const uint8_t FONTSET_SIZE = 80;
    const static uint8_t FONTSET[80] = { 
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

void cchip8_step(cchip8_context_t* self, uint32_t update_rate)
{
    if(!self->cpu.interpreter.running) return;

    long start_time, end_time;
    if(self->speed == (uint32_t)-1)
    {
        start_time = thread_get_nanos();
        chip8_interpreter_step(&self->cpu.interpreter);
        end_time = thread_get_nanos();
        chip8_interpreter_update_timers(&self->cpu.interpreter, end_time - start_time + rand() % 100);   // rand() % 100 is a trick cuz thread_get_nanos() is not accurate enough.
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

static int cpu_thread_function(void* args)
{
    cchip8_context_t* const self = args;
    while(true)
    {
        cchip8_step(self, 75);

        if(self->speed != (uint32_t)-1)
            thread_sleep(SDL_NS_PER_SECOND / 75);

        if(!self->cpu.interpreter.running) break;
    }
    fprintf(stderr, "[INFO]: Emulator has stopped.\n");
    return 0;
}

void cchip8_run_sdl(cchip8_context_t *self, ini_file_t *config, bool threaded)
{
    // Init SDL subsystem.
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        cchip8_free(self);
        fprintf(stderr, "[EROR]: Could not initialize SDL! %s\n", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not initialize SDL3!", NULL);
        return;
    }
    char* const rom_path = ini_data_get_as_string(ini_file_get_data(config, "chip8.core", "path"));
    const char* app_title = "cchip8 | ";
    char window_title[strlen(rom_path) + strlen(app_title) + 1];
    string_concat((char* const)window_title, (char* const)app_title, (char* const)rom_path);
    SDL_Window* window = SDL_CreateWindow(window_title, 960, 540, SDL_WINDOW_RESIZABLE);
    if(window == NULL)
    {
        cchip8_free(self);
        fprintf(stderr, "[EROR]: Could not create SDL3 window! %s\n", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not create SDL3 window!", NULL);
        return;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if(renderer == NULL)
    {
        cchip8_free(self);
        fprintf(stderr, "[EROR]: Could not create SDL3 renderer! %s\n", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not create SDL3 renderer!", window);
        SDL_DestroyWindow(window);
        return;
    }
    SDL_SetRenderLogicalPresentation(renderer, 64, 32, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Create CPU thread.
    SDL_Thread* cpu_thread;
    if(threaded)
    {
        cpu_thread = SDL_CreateThread(cpu_thread_function, "c8 cpu thr", self);
        if(cpu_thread == NULL)
        {
            cchip8_free(self);
            fprintf(stderr, "[EROR]: Could not create emulator thread!\n");
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            return;
        }
    }

    // Main loop.
    SDL_Event event;
    bool app_running = true;
    ini_data_t* const foreground_array = ini_file_get_data(config, "chip8.output.sdl.colors", "foreground");
    ini_data_t* const background_array = ini_file_get_data(config, "chip8.output.sdl.colors", "background");
    struct SDL_Color foreground, background;
    foreground.r = ini_data_get_as_int(ini_data_get_from_array(foreground_array, 0));
    foreground.g = ini_data_get_as_int(ini_data_get_from_array(foreground_array, 1));
    foreground.b = ini_data_get_as_int(ini_data_get_from_array(foreground_array, 2));
    background.r = ini_data_get_as_int(ini_data_get_from_array(background_array, 0));
    background.g = ini_data_get_as_int(ini_data_get_from_array(background_array, 1));
    background.b = ini_data_get_as_int(ini_data_get_from_array(background_array, 2));

    while(app_running)  // No need to sync here, running one frame more than the CPU can't hurt.
    {
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_EVENT_QUIT:
                app_running = false;
                break;
            }
        }

        if(!threaded)
            cchip8_step(self, 60);

        SDL_SetRenderDrawColor(renderer, background.r, background.g, background.b, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, foreground.r, foreground.g, foreground.b, 255);
        
        if(threaded)
            SDL_LockRWLockForReading(self->display_lock);
        for(uint8_t x = 0; x < 64; x++)
            for(uint8_t y = 0; y < 32; y++)
                if(bitset_get(&self->display_memory, x + y * 64))
                    SDL_RenderPoint(renderer, x, y);
        if(threaded)
            SDL_UnlockRWLock(self->display_lock);
        SDL_RenderPresent(renderer);
        thread_sleep(SDL_NS_PER_SECOND / 60);
    }

    if(threaded)
    {
        self->cpu.interpreter.running = false;
        SDL_WaitThread(cpu_thread, NULL); 
    }

    cchip8_free(self);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void cchip8_free(cchip8_context_t* self)
{
    bitset_free(&self->display_memory);
    SDL_DestroyRWLock(self->display_lock);
}
