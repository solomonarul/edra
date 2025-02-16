#define _CRT_SECURE_NO_WARNINGS     // Windows fix for being annoying.
#include <SDL3/SDL.h>
#include <threads.h>
#include <assert.h>
#include <stdlib.h>

#include <auxum/unused.h>
#include <auxum/strings.h>
#include <drivers/chip8.h>

int cpu_thread_function(void* args)
{
    cchip8_run(args);
    return 0;
}

int main(int argc, char* argv[])
{
    UNUSED(argc); UNUSED(argv);
    
    // Init the emulator.
    cchip8_context_t emulator;
    cchip8_init(&emulator);

    // Load the ROM.
    const char* rom_path = "roms/chip8/games/Tetris [Fran Dachille, 1991].ch8";
    FILE* rom = fopen(rom_path, "rb");
    if(rom == NULL)
    {
        cchip8_free(&emulator);
        fprintf(stderr, "[EROR]: Could not find ROM file at path %s!", rom_path);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not load specified ROM file!", NULL);
        return -1;
    }
    fread((char*)emulator.memory + emulator.state.pc, sizeof(uint8_t), 0x10000 - emulator.state.pc, rom);
    fclose(rom);

    // Init SDL subsystem.
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        cchip8_free(&emulator);
        fprintf(stderr, "[EROR]: Could not initialize SDL! %s", SDL_GetError());
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
        fprintf(stderr, "[EROR]: Could not create SDL3 window! %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not create SDL3 window!", NULL);
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if(renderer == NULL)
    {
        cchip8_free(&emulator);
        fprintf(stderr, "[EROR]: Could not create SDL3 renderer! %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not create SDL3 renderer!", window);
        SDL_DestroyWindow(window);
        return -1;
    }
    SDL_SetRenderLogicalPresentation(renderer, 64, 32, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Create CPU thread.
    thrd_t cpu_thread;
    if(thrd_create(&cpu_thread, cpu_thread_function, &emulator) != thrd_success)
    {
        cchip8_free(&emulator);
        fprintf(stderr, "[EROR]: Could not create emulator thread!");
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
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
        thrd_sleep(&(struct timespec){.tv_nsec=16666666}, NULL);    // TODO: not really accurate let's implement our own spinlocker later.
    }

    emulator.cpu->running = false;
    thrd_join(cpu_thread, NULL);

    cchip8_free(&emulator);
    SDL_Quit();
    
    return 0;
}
