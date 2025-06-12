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

    // Create CHIP8 emulator.
    chip8_app_state_t chip8_state = {0};
    chip8_app_state_init(&chip8_state);
    maybe_t load_rom_result = chip8_app_state_load_rom(
        &chip8_state, CHIP8_MODE_NORMAL,
#ifdef BUILD_TYPE_VITA
        fopen("ux0:/shared/roms/c8/games/Tetris [Fran Dachille, 1991].ch8", "rb"), 
#else
        fopen("./roms/c8/games/Tetris [Fran Dachille, 1991].ch8", "rb"), 
#endif
        true, true
    );
    if(!IS_OK(load_rom_result))
        printf("[WARN]: Couldn't load ROM: %s\n", load_rom_result.error);
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
        "Edra | cCHIP8"
    });
    if(!IS_OK(result))
    {
        app_window_free(&window);
        app_show_sdl_error(result.error, NULL);
    }
    app_window_enable_vsync(&window, true);

    app_run_main_loop(&window);
    
    app_state_free();
    app_window_free(&window);
    SDL_Quit();
    return 0;
}
