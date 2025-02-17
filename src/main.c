#define _CRT_SECURE_NO_WARNINGS     // Windows fix for being annoying.
#include <SDL3/SDL.h>
#include <assert.h>
#include <stdlib.h>

#include <auxum/unused.h>
#include <auxum/thread.h>
#include <auxum/strings.h>
#include <drivers/chip8.h>

int cpu_thread_function(void* args)
{
    cchip8_run(args);
    return 0;
}

bool get_key_status(void* arg, uint8_t key)
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
    assert(key < 0x10);
    return keyboard_state[keys[key]];
}

int main(int argc, char* argv[])
{
    UNUSED(argc); UNUSED(argv);
    
    // Init the emulator.
    cchip8_context_t emulator;
    cchip8_init(&emulator);
    emulator.speed = 600;
    emulator.state.get_key_status = get_key_status;

    // Load the ROM.
    const char* rom_path = "roms/chip8/demos/heart_monitor.ch8";
    FILE* rom = fopen(rom_path, "rb");
    if(rom == NULL)
    {
        cchip8_free(&emulator);
        fprintf(stderr, "[EROR]: Could not find ROM file at path %s!\n", rom_path);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not load specified ROM file!", NULL);
        return -1;
    }
    fread((char*)emulator.memory + emulator.state.pc, sizeof(uint8_t), 0x10000 - emulator.state.pc, rom);
    fclose(rom);
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
    };
    memcpy((char*)emulator.memory, FONTSET, sizeof(uint8_t) * FONTSET_SIZE);

    // Init SDL subsystem.
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        cchip8_free(&emulator);
        fprintf(stderr, "[EROR]: Could not initialize SDL! %s\n", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not initialize SDL3!", NULL);
        return -1;
    }
    const char* app_title = "cchip8 | ";
    char window_title[strlen(rom_path) + strlen(app_title) + 1];
    string_concat((char* const)window_title, (char* const)app_title, (char* const)rom_path);
    SDL_Window* window = SDL_CreateWindow(window_title, 960, 540, SDL_WINDOW_RESIZABLE);
    if(window == NULL)
    {
        cchip8_free(&emulator);
        fprintf(stderr, "[EROR]: Could not create SDL3 window! %s\n", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not create SDL3 window!", NULL);
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if(renderer == NULL)
    {
        cchip8_free(&emulator);
        fprintf(stderr, "[EROR]: Could not create SDL3 renderer! %s\n", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not create SDL3 renderer!", window);
        SDL_DestroyWindow(window);
        return -1;
    }
    SDL_SetRenderLogicalPresentation(renderer, 64, 32, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Create CPU thread.
    SDL_Thread* cpu_thread = SDL_CreateThread(cpu_thread_function, "c8 cpu thr", &emulator);
    if(cpu_thread == NULL)
    {
        cchip8_free(&emulator);
        fprintf(stderr, "[EROR]: Could not create emulator thread!\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return -1;
    }

    // Main loop.
    SDL_Event event;
    bool app_running = true;
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

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        // TODO: RWLock this.
        for(uint8_t x = 0; x < 64; x++)
            for(uint8_t y = 0; y < 32; y++)
                if(bitset_get(&emulator.display_memory, x + y * 64))
                    SDL_RenderPoint(renderer, x, y);
        SDL_RenderPresent(renderer);
        thread_sleep(1e9 / 60);
    }

    emulator.cpu.interpreter.running = false;
    SDL_WaitThread(cpu_thread, NULL);

    cchip8_free(&emulator);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
