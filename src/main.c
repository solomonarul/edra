#define _CRT_SECURE_NO_WARNINGS
#include <SDL3/SDL.h>
#include <threads.h>

#include "auxum/bitset.h"
#include "auxum/strings.h"
#include "cchip8/state.h"
#include "cchip8/memory.h"
#include "cchip8/cpu/interpreter.h"

#define UNUSED(X) ((void)(X))

static uint8_t memory[0x10000];
static bitset_t display_memory;

void clear_screen()
{
    bitset_clear(&display_memory);
}

bool draw_sprite(uint16_t address, uint8_t x, uint8_t y, uint8_t n)
{
    bool collision = false;
    for(uint16_t index = address; index < address + n; index++)
    {
        const uint8_t byte = memory[index];
        for(uint8_t bit = 0; bit < 8; bit++)
        {
            const int position = x + (7 - bit) + (y + (index - address)) * 64;
            const bool pixel = (byte & (1 << bit)) != 0;
            if(bitset_get(display_memory, position) == pixel) collision = true;
            bitset_xor(&display_memory, position, pixel);
        }
    }
    return collision;
}

int cpu_thread_function(void* args)
{
    const int speed = -1;
    chip8_interpreter_t* cpu = (chip8_interpreter_t*)args;

    while(true)
    {
        chip8_interpreter_step(cpu);
        if(!cpu->running) break;

        if(speed > 0)
            thrd_sleep(&(struct timespec){.tv_nsec=1000000000 / speed}, NULL);    // TODO: not really accurate let's implement our own spinlocker later.
    }
    fprintf(stderr, "[INFO]: Emulator has stopped.");

    return 0;
}

int main(int argc, char* argv[])
{
    UNUSED(argc); UNUSED(argv);

    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "[EROR]: Could not init SDL!");
        return -1;
    }

    chip8_state_t state;
    chip8_state_init(&state);
    state.draw_sprite = draw_sprite;
    state.clear_screen = clear_screen;
    chip8_memory_set_base(memory);
    bitset_init(&display_memory, 64 * 32);

    chip8_interpreter_t cpu;
    chip8_interpreter_init(&cpu);
    cpu.running = true;
    cpu.state = &state;

    const char* rom_path = "roms/chip8/timendus/3-corax+.ch8";
    FILE* rom = fopen(rom_path, "rb");
    fread((char*)memory + state.pc, sizeof(uint8_t), 0x10000 - state.pc, rom);
    fclose(rom);

    const char* app_title = "cchip8 | ";
    char window_title[strlen(rom_path) + strlen(app_title) + 1];
    string_concat((char* const)window_title, (char* const)app_title, (char* const)rom_path);
    SDL_Window* window = SDL_CreateWindow(window_title, 960, 540, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    SDL_SetRenderLogicalPresentation(renderer, 64, 32, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    thrd_t cpu_thread;
    if(thrd_create(&cpu_thread, cpu_thread_function, &cpu) != thrd_success)
    {
        fprintf(stderr, "[EROR]: Could not create emulator thread!");
        return -1;
    }

    // No need to sync, running one frame more than the CPU can't hurt.
    SDL_Event event;
    bool app_running = true;
    while(app_running)
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

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        // TODO: RWLock this.
        for(uint8_t x = 0; x < 64; x++)
            for(uint8_t y = 0; y < 32; y++)
                if(bitset_get(display_memory, x + y * 64))
                    SDL_RenderPoint(renderer, x, y);
        SDL_RenderPresent(renderer);
        thrd_sleep(&(struct timespec){.tv_nsec=16666666}, NULL);    // TODO: not really accurate let's implement our own spinlocker later.
    }

    cpu.running = false;
    thrd_join(cpu_thread, NULL);

    SDL_Quit();
    
    return 0;
}
