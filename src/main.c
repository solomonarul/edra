#include "auxum/std/error.h"
#include <auxum/std.h>
#include <auxum/file/ini.h>
#include <drivers/chip8.h>

// Error handler.
static void show_error(char* const error)
{
    fprintf(stderr, "[EROR] %s\n", error);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", error, NULL);
}

int main(int argc, char* argv[])
{
    ini_file_result_t config_result;

    // No config, cry.
    if(argc < 2 || !IS_OK(config_result = ini_file_read(argv[1]))) {
        if(argc < 2) { config_result.error = "No INI file specified!"; }
        show_error(RESULT_GET_ERROR(config_result));
        return -1;
    }
    
    // Run the emulator.
    ini_file_t* const config = &RESULT_GET_VALUE(config_result);
    ini_string_result_t type_result = ini_get_string(config, "system", "module");
    if(!IS_OK(type_result))
    {
        show_error("No module specified to run!");
        return -1;
    }
    char* const type = RESULT_GET_VALUE(type_result);
    if(strcmp(type, "chip8") == 0)
    {
        maybe_t result = cchip8_run_from_ini(&RESULT_GET_VALUE(config_result));
        ini_file_free(&RESULT_GET_VALUE(config_result));
        if(!IS_OK(result)) {
            show_error(RESULT_GET_ERROR(result));
            return -1;
        }
        return 0;
    }
    show_error("Invalid module type specified!");
    return -1;
}
