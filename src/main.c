#ifdef BUILD_TYPE_WINDOWS
#define _CRT_SECURE_NO_WARNINGS     // Windows fix for being annoying.
#endif
#include <SDL3/SDL.h>
#include <assert.h>
#include <stdlib.h>

#include <auxum/ini.h>
#include <auxum/unused.h>
#include <auxum/thread.h>
#include <auxum/strings.h>
#include <drivers/chip8.h>

bool sdl_get_key_status(void* arg, uint8_t key)
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

int main(int argc, char* argv[])
{
    // No config, cry.
    if(argc < 2) {
        fprintf(stderr, "[EROR]: No INI file specified!\n");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "No INI file specified!\n", NULL);
        return -1;
    }

    // Parse the config.
    FILE* input = fopen(argv[1], "r");
    ini_file_t config = ini_file_parse(input);
    fclose(input);
    
    // Init the emulator.
    cchip8_context_t emulator;
    cchip8_init(&emulator);
    emulator.speed = ini_data_get_as_int(ini_file_get_data(&config, "chip8.core", "speed"));
    if(strcmp(ini_data_get_as_string(ini_file_get_data(&config, "chip8.core", "input")), "sdl") == 0)
        emulator.state.get_key_status = sdl_get_key_status;

    // Load the ROM.
    char* const rom_path = ini_data_get_as_string(ini_file_get_data(&config, "chip8.core", "path"));
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

    if(strcmp(ini_data_get_as_string(ini_file_get_data(&config, "chip8.core", "output")), "sdl") == 0)
        cchip8_run_sdl(&emulator, &config, ini_data_get_as_bool(ini_file_get_data(&config, "chip8.core", "threaded")));
    
    return 0;
}
