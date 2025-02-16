#define _CRT_SECURE_NO_WARNINGS     // Windows fix for being annoying.
#include <SDL3/SDL.h>
#include <threads.h>
#include <assert.h>
#include <stdlib.h>

#include "auxum/bitset.h"
#include "auxum/strings.h"
#include "cchip8/state.h"
#include "cchip8/memory.h"
#include "cchip8/cpu/interpreter.h"

#define UNUSED(X) ((void)(X))

const char* app_title = "cchip8 | ";
static uint8_t memory[0x10000];
static bitset_t display_memory; // TODO: RWLock this otherwise you get half-drawn sprites.

void clear_screen()
{
    bitset_clear(&display_memory);
}

bool draw_sprite(uint16_t address, uint8_t x, uint8_t y, uint8_t n)
{
    x %= 64;
    y %= 32;
    bool collision = false;
    for(uint16_t index = address; index < address + n; index++)
    {
        const uint8_t byte = memory[index];
        for(uint8_t bit = 0; bit < 8; bit++)
        {
            const uint8_t new_x = x + (7 - bit);
            const uint8_t new_y = y + (index - address);
            if(new_x > 63 || new_y > 31) continue;
            const int position = new_x + new_y * 64;
            const bool pixel = (byte & (1 << bit)) != 0;
            if(pixel == true && bitset_get(display_memory, position) == pixel) collision = true;
            bitset_xor(&display_memory, position, pixel);
        }
    }
    return collision;
}

bool get_key_status(uint8_t key)
{
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

uint8_t get_random()
{
    return rand();
}

int cpu_thread_function(void* args)
{
    const int speed = 1000;
    chip8_interpreter_t* cpu = (chip8_interpreter_t*)args;

    while(true)
    {
        chip8_interpreter_step(cpu);
        chip8_interpreter_update_timers(cpu, 1.0 / speed);
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
        fprintf(stderr, "[EROR]: Could not initialize SDL! %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not initialize SDL3!", NULL);
        return -1;
    }

    chip8_state_t state;
    chip8_state_init(&state);
    state.draw_sprite = draw_sprite;
    state.clear_screen = clear_screen;
    state.get_key_status = get_key_status;
    state.get_random = get_random;
    chip8_memory_set_base(memory);
    bitset_init(&display_memory, 64 * 32);

    chip8_interpreter_t cpu;
    chip8_interpreter_init(&cpu);
    cpu.running = true;
    cpu.state = &state;

    const char* rom_path = "roms/chip8/games/Tetris [Fran Dachille, 1991].ch8";
    FILE* rom = fopen(rom_path, "rb");
    if(rom == NULL)
    {
        bitset_free(&display_memory);
        fprintf(stderr, "[EROR]: Could not find ROM file at path %s!", rom_path);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not load specified ROM file!", NULL);
        return -1;
    }
    fread((char*)memory + state.pc, sizeof(uint8_t), 0x10000 - state.pc, rom);
    fclose(rom);    

    char window_title[strlen(rom_path) + strlen(app_title) + 1];
    string_concat((char* const)window_title, (char* const)app_title, (char* const)rom_path);
    SDL_Window* window = SDL_CreateWindow(window_title, 960, 540, SDL_WINDOW_RESIZABLE);
    if(window == NULL)
    {
        bitset_free(&display_memory);
        fprintf(stderr, "[EROR]: Could not create SDL3 window! %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not create SDL3 window!", NULL);
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if(renderer == NULL)
    {
        bitset_free(&display_memory);
        fprintf(stderr, "[EROR]: Could not create SDL3 renderer! %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not create SDL3 renderer!", window);
        SDL_DestroyWindow(window);
        return -1;
    }
    SDL_SetRenderLogicalPresentation(renderer, 64, 32, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    thrd_t cpu_thread;
    if(thrd_create(&cpu_thread, cpu_thread_function, &cpu) != thrd_success)
    {
        bitset_free(&display_memory);
        fprintf(stderr, "[EROR]: Could not create emulator thread!");
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
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
