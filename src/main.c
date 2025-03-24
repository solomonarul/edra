#include <auxum/std.h>
#include <auxum/file/ini.h>
#include "system/window.h"
#include "drivers/chip8.h"

int main(int argc, char* argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    
    // Create window.
    app_window_t window = {0};
    maybe_t result = app_window_init(&window, &(app_window_init_data_t) {
    #ifdef BUILD_TYPE_VITA
        .size_x = 960,
        .size_y = 544,
        .flags = SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS,
    #else
        .size_x = 640,
        .size_y = 320,
        .flags = SDL_WINDOW_RESIZABLE,
    #endif
        "Edra | cCHIP8 no-rom"
    });
    if(!result.ok) {
        app_window_free(&window);
        app_show_sdl_error(result.error, NULL);
    }
    SDL_SetRenderVSync(window.renderer, 1);

    // Create CHIP8 emulator.
    cchip8_context_t emulator;
    cchip8_init(&emulator);
    emulator.state.get_key_status = cchip8_get_sdl_key_status;
    emulator.speed = 60;
    /*FILE* rom = fopen("./roms/chip8/schip/games/octogon.ch8", "rb");
    if(rom == NULL)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        show_error("CHIP8 ROM file does not exist!");
    }
    fread(self.memory + 0x200, sizeof(uint8_t), 0x1000 - self.state.pc, rom);
    self.state.mode = CHIP8_MODE_SCHIP_MODERN;*/
    uint8_t norom[62] =
    {
        0x61, 0x0e, 0x62, 0x0d, 0xa2, 0x22, 0xd1, 0x27, 
        0x71, 0x08, 0xa2, 0x29, 0xd1, 0x27, 0x71, 0x0a, 
        0xa2, 0x30, 0xd1, 0x27, 0x71, 0x06, 0xa2, 0x29, 
        0xd1, 0x27, 0x71, 0x05, 0xa2, 0x37, 0xd1, 0x27, 
        0x12, 0x20, 0x82, 0xc2, 0xa2, 0x92, 0x8a, 0x86, 
        0x82, 0x00, 0x00, 0x60, 0x90, 0x90, 0x90, 0x60, 
        0xf0, 0x88, 0x88, 0x88, 0xd0, 0xa0, 0x90, 0x00, 
        0x00, 0x88, 0xd8, 0xa8, 0x88, 0x88, 
    };
    memcpy(emulator.memory + 0x200, norom, 62 * sizeof(uint8_t));

    // Create CPU thread.
    SDL_Thread* cpu_thread = SDL_CreateThread(cchip8_cpu_thread_function, "c8 cpu thr", &emulator);
    if(cpu_thread == NULL)
    {
        cchip8_free(&emulator);
        app_window_free(&window);
        app_show_sdl_error("Could not create CHIP8 emulator thread!", NULL);
    }
    emulator.threaded = true;

    // Main loop.
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
            case SDL_EVENT_WINDOW_RESIZED:
                app_window_on_resize(&window, event.window.data1, event.window.data2);
                break;
            }
        }
        cchip8_draw_sdl(&emulator, window.renderer);
        SDL_RenderPresent(window.renderer);
    }

    emulator.cpu.interpreter.running = false;
    SDL_WaitThread(cpu_thread, NULL); 

    cchip8_free(&emulator);
    app_window_free(&window);
    SDL_Quit();
    return 0;
}
