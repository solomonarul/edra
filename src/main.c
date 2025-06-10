#include <auxum/std.h>
#include "system/window.h"
#include "drivers/bf.h"
#include "states/chip8_state.h"

#include <time.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    auxum_set_app_path(argv[0]);
    srand(time(NULL));

    UNUSED(argc);
    UNUSED(argv);

    app_state_init();

    // Create BF emulator.
    /*FILE* program = fopen("./roms/bf/mandlebrot.b", "r");
    cbf_context_t bf_emulator = {0};
    cbf_init(&bf_emulator, BF_RUN_INTERPRETER);
    size_t rom_size = cbf_read(&bf_emulator, program, BF_OPTIMIZATIONS_ALL);
    fclose(program);

    printf("[INFO]: Brainfuck program size: %d instructions.\n", (int)rom_size);

    uint64_t begin = SDL_GetPerformanceCounter();

    while(cbf_is_running(&bf_emulator))
        cbf_step(&bf_emulator);

    uint64_t end = SDL_GetPerformanceCounter();
    double time_spent = (double)(end - begin) * 1000 / SDL_GetPerformanceFrequency();

    printf("[INFO]: Brainfuck program spent %lfms running (%d SDL_GetPerformance clocks).\n", time_spent, (int)(end - begin));

    cbf_free(&bf_emulator);*/

    // Create CHIP8 emulator.
    chip8_app_state_t chip8_state = {0};
    chip8_app_state_init(&chip8_state);
    maybe_t load_rom_result = chip8_app_state_load_rom(
        &chip8_state, CHIP8_MODE_NORMAL,
        fopen("./roms/c8/games/Tetris [Fran Dachille, 1991].ch8", "rb"), 
        true, true
    );
    if(!IS_OK(load_rom_result))
        app_show_error(load_rom_result.error);
    app_state_push(chip8_state.internal);
    
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
    if(!IS_OK(result))
    {
        app_window_free(&window);
        app_show_sdl_error(result.error, NULL);
    }
    app_window_enable_vsync(&window, true);

    // Main loop.
    SDL_Event event;
    bool app_running = true;
    long start_time, end_time;
    start_time = end_time = THREAD_NANOS;
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
        end_time = THREAD_NANOS;
        app_update(end_time - start_time);
        start_time = end_time;
        app_render(&window);
        SDL_RenderPresent(window.renderer);
    }
    app_state_free();
    app_window_free(&window);
    SDL_Quit();
    return 0;
}
