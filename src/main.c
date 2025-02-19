#ifdef BUILD_TYPE_WINDOWS
#define _CRT_SECURE_NO_WARNINGS     // Windows fix for being annoying.
#endif
#include <SDL3/SDL.h>
#include <assert.h>
#include <stdlib.h>

#include <auxum/std.h>
#include <auxum/file/ini.h>
#include <auxum/os/thread.h>
#include <drivers/chip8.h>

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
    emulator.speed = ini_get_int(&config, "chip8.core", "speed");
    if(strcmp(ini_get_string(&config, "chip8.core", "input"), "sdl") == 0)
        emulator.state.get_key_status = cchip8_sdl_get_key_status;

    // Load the ROM.
    char* const rom_path = ini_get_string(&config, "chip8.core", "path");
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

    if(strcmp(ini_get_string(&config, "chip8.core", "output"), "sdl") == 0)
        cchip8_run_sdl(&emulator, &config, ini_get_bool(&config, "chip8.core", "threaded"));
    
    return 0;
}
