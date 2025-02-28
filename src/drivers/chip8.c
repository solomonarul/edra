#include "drivers/chip8.h"
#include <auxum/std.h>
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <assert.h>

static char* const CHIP8_ERROR_INI_NO_CORE_SECTION = "No chip8.core section in INI file!";

uint8_t memory_read_b(void* arg, uint16_t address)
{
    cchip8_context_t* const self = arg;
    return self->memory[address];
}

uint16_t memory_read_w(void* arg, uint16_t address)
{
    cchip8_context_t* const self = arg;
    return (self->memory[address] << 8) | self->memory[address + 1]; 
}

void memory_write_b(void* arg, uint16_t address, uint8_t value)
{
    cchip8_context_t* const self = arg;
    self->memory[address] = value;
}

void clear_screen(void* arg)
{
    cchip8_context_t* const self = arg;
    SDL_LockRWLockForWriting(self->display_lock);
    bitset_clear(&self->display_memory);
    SDL_UnlockRWLock(self->display_lock);
}

bool draw_sprite(void* arg, uint16_t address, uint8_t x, uint8_t y, uint8_t n)
{
    cchip8_context_t* const self = arg;
    x %= 64;
    y %= 32;
    bool collision = false;
    SDL_LockRWLockForWriting(self->display_lock);
    for(uint16_t index = address; index < address + n; index++)
    {
        const uint8_t byte = self->memory[index];
        for(uint8_t bit = 0; bit < 8; bit++)
        {
            const uint8_t new_x = x + (7 - bit);
            const uint8_t new_y = y + (index - address);
            if(new_x > 63 || new_y > 31) continue;
            const int position = new_x + new_y * 64;
            const bool pixel = (byte & (1 << bit)) != 0;
            if(pixel == true && bitset_get(&self->display_memory, position) == pixel) collision = true;
            bitset_xor(&self->display_memory, position, pixel);
        }
    }
    SDL_UnlockRWLock(self->display_lock);
    return collision;
}

uint8_t get_random(void* arg)
{
    UNUSED(arg);
    return rand();
}

static void cchip8_free(cchip8_context_t* self)
{
    bitset_free(&self->display_memory);
    SDL_DestroyRWLock(self->display_lock);
}

void cchip8_init(cchip8_context_t* self)
{
    // Setup state and callbacks.
    chip8_state_init(&self->state);
    self->state.read_b = memory_read_b;
    self->state.read_w = memory_read_w;
    self->state.write_b = memory_write_b;
    self->state.draw_sprite = draw_sprite;
    self->state.clear_screen = clear_screen;
    self->state.get_random = get_random;
    self->state.aux_arg = self;
    bitset_init(&self->display_memory, 64 * 32);
    self->display_lock = SDL_CreateRWLock();

    // Setup CPU.
    chip8_interpreter_init(&self->cpu.interpreter);
    self->cpu.interpreter.running = true;
    self->cpu.interpreter.state = &self->state;
    self->speed = -1;

    const uint8_t FONTSET_SIZE = 80;
    static const uint8_t FONTSET[80] = { 
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
    };  // TODO: make this modifiable from outside?
    memcpy((char*)self->memory, FONTSET, sizeof(uint8_t) * FONTSET_SIZE);
}

#define THREAD_NANOS (SDL_GetPerformanceCounter() * 1.0 / SDL_GetPerformanceFrequency() * SDL_NS_PER_SECOND)

void cchip8_step(cchip8_context_t* self, uint32_t update_rate)
{
    if(!self->cpu.interpreter.running) return;

    long start_time, end_time;
    if(self->speed == (uint32_t)-1)
    {
        start_time = THREAD_NANOS;
        chip8_interpreter_step(&self->cpu.interpreter);
        end_time = THREAD_NANOS;
        chip8_interpreter_update_timers(&self->cpu.interpreter, end_time - start_time + rand() % 100);   // rand() % 100 is a trick cuz THREAD_NANOS is not accurate enough.
    }
    else {
        start_time = 0;
        while(start_time < SDL_NS_PER_SECOND / update_rate)
        {
            chip8_interpreter_step(&self->cpu.interpreter);
            chip8_interpreter_update_timers(&self->cpu.interpreter, SDL_NS_PER_SECOND / self->speed);
            if(!self->cpu.interpreter.running) return;
            start_time += SDL_NS_PER_SECOND / self->speed;
        }
    }
}

static int cpu_thread_function(void* args)
{
    cchip8_context_t* const self = args;
    while(true)
    {
        if(self->speed != (uint32_t)-1)
        {
            cchip8_step(self, 75);
            SDL_DelayNS(SDL_NS_PER_SECOND / 75);
        }
        else
            cchip8_step(self, SDL_NS_PER_SECOND);

        if(!self->cpu.interpreter.running) break;
    }
    fprintf(stdout, "[CHP8] Emulator has stopped.\n");
    return 0;
}

bool cchip8_sdl_get_key_status(void* args, uint8_t key)
{
    cchip8_context_t* const self = args;
    static const bool* keyboard_state;
    keyboard_state = SDL_GetKeyboardState(NULL);
    for(uint32_t index = 0; index < self->key_mappings[key].size; index++)
        if(keyboard_state[*(SDL_Scancode*)dynarray_get(self->key_mappings[key], index)])
            return true;
    return false;
}

bool cchip8_none_get_key_status(void* args, uint8_t key)
{
    UNUSED(args); UNUSED(key);
    return false;
}

maybe_t cchip8_run_none(cchip8_context_t *self, ini_file_t *config)
{
    maybe_t result;
    fprintf(stdout, "[CHP8] Output: none.\n");

    // Create CPU thread.
    ini_section_ptr_result_t core_section_result = ini_file_get_section(config, "chip8.core");
    if(!IS_OK(core_section_result))
    {
        result.ok = false;
        result.error = CHIP8_ERROR_INI_NO_CORE_SECTION;
        return result;
    }
    ini_section_t* const core_section = core_section_result.result;
    ini_data_ptr_result_t threaded_data_result = ini_section_get_data(core_section, "threaded");
    if(!IS_OK(threaded_data_result)) threaded_data_result.result = NULL;
    bool threaded = RESULT_GET_OR(ini_data_get_as_bool(threaded_data_result.result), false);
    SDL_Thread* cpu_thread = NULL;
    if(threaded)
    {
        cpu_thread = SDL_CreateThread(cpu_thread_function, "c8 cpu thr", self);
        if(cpu_thread == NULL)
        {
            cchip8_free(self);
            result.ok = false;
            result.error = "Could not create emulator thread!";
            return result;
        }
    }

    // Main loop.
    bool app_running = true;
    while(app_running)  // No need to sync here, running one frame more than the CPU can't hurt.
    {
        if(!threaded)
            cchip8_step(self, 60);
        SDL_DelayNS(SDL_NS_PER_SECOND / 60);
    }

    if(threaded)
    {
        self->cpu.interpreter.running = false;
        SDL_WaitThread(cpu_thread, NULL); 
    }

    cchip8_free(self);
    result.ok = true;
    return result;
}

void cchip8_run_sdl(cchip8_context_t *self, ini_file_t *config)
{
    fprintf(stdout, "[CHP8] Output: SDL.\n");

    // Convert the key mappings to SDL keys.
    fprintf(stdout, "[CHP8] SDL keys: ");
    ini_section_ptr_result_t input_section_result = ini_file_get_section(config, "chip8.input.sdl.input");
    if(!IS_OK(input_section_result))
    {
        fprintf(stderr, "[CHIP8] [WARN] No SDL Key section found, using default mapping!\n");
        fprintf(stderr, "TODO: Implement this.\n");
    }
    else {
        ini_section_t* const input_section = input_section_result.result;
        ini_data_ptr_result_t input_data_result = ini_section_get_data(input_section, "keys");
        if(!IS_OK(input_data_result))
        {
            fprintf(stderr, "[CHIP8] [WARN] No SDL Key listing found, using default mapping!\n");
            fprintf(stderr, "TODO: Implement this.\n");
        }
        ini_data_t* const input_data = input_data_result.result;
        for(uint32_t index = 0; index < 0x10; index++)
        {
            fprintf(stdout, "[");
            dynarray_init(&self->key_mappings[index], sizeof(SDL_Scancode), 0);
            ini_data_ptr_result_t key_array_result = ini_data_get_from_array(input_data, index);
            if(!IS_OK(key_array_result))
            {
                fprintf(stderr, "[CHIP8] [WARN] Ignoring key %d as its data is not valid!\n", index);
                continue;
            }
            ini_data_t* const key_array = key_array_result.result;
            for(int index_key = 0; index_key < ini_data_get_array_size(key_array); index_key++)
            {
                // This should be always valid.
                char* const key = ini_data_get_as_string(ini_data_get_from_array(key_array, index_key).result).result;
                SDL_Scancode code = SDL_GetScancodeFromKey(SDL_GetKeyFromName(key), NULL);
                if(code != SDL_SCANCODE_UNKNOWN)
                {
                    fprintf(stdout, "%s", key);
                    dynarray_push_back(&self->key_mappings[index], &code);
                }
            }
            if(index != 0xF)
                fprintf(stdout, "], ");
            else
                fprintf(stdout, "]");
        }
        fprintf(stdout, "\n");
    }

    // Init SDL subsystem.
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        cchip8_free(self);
        fprintf(stderr, "[EROR] Could not initialize SDL! %s\n", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not initialize SDL3!", NULL);
        return;
    }
    ini_section_t* const core_section = ini_file_get_section(config, "chip8.core").result;
    char* const rom_path = ini_data_get_as_string(ini_section_get_data(core_section, "path").result).result;
    const char* app_title = "cchip8 | ";
    char window_title[strlen(rom_path) + strlen(app_title) + 1];
    string_concat((char* const)window_title, (char* const)app_title, (char* const)rom_path);
    SDL_Window* window = SDL_CreateWindow(window_title, 640, 320, SDL_WINDOW_RESIZABLE);
    if(window == NULL)
    {
        cchip8_free(self);
        fprintf(stderr, "[EROR] Could not create SDL3 window! %s\n", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not create SDL3 window!", NULL);
        return;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if(renderer == NULL)
    {
        cchip8_free(self);
        fprintf(stderr, "[EROR] Could not create SDL3 renderer! %s\n", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Could not create SDL3 renderer!", window);
        SDL_DestroyWindow(window);
        return;
    }
    SDL_SetRenderLogicalPresentation(renderer, 64, 32, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Create CPU thread.
    bool threaded = RESULT_GET_OR(ini_data_get_as_bool(ini_section_get_data(core_section, "threaded").result), false);
    SDL_Thread* cpu_thread = NULL;
    if(threaded)
    {
        cpu_thread = SDL_CreateThread(cpu_thread_function, "c8 cpu thr", self);
        if(cpu_thread == NULL)
        {
            cchip8_free(self);
            fprintf(stderr, "[EROR] Could not create emulator thread!\n");
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            return;
        }
    }

    // Color parsing.
    ini_data_t* const foreground_array = ini_file_get_data(config, "chip8.output.sdl.output", "foreground").result;
    ini_data_t* const background_array = ini_file_get_data(config, "chip8.output.sdl.output", "background").result;
    struct SDL_Color foreground, background;
    if(ini_data_get_array_size(foreground_array) == 3)
    {
        foreground.r = ini_data_get_as_int(ini_data_get_from_array(foreground_array, 0).result).result;
        foreground.g = ini_data_get_as_int(ini_data_get_from_array(foreground_array, 1).result).result;
        foreground.b = ini_data_get_as_int(ini_data_get_from_array(foreground_array, 2).result).result;
    }
    else {
        foreground.r = 200;
        foreground.g = 200;
        foreground.b = 200;
    }
    if(ini_data_get_array_size(background_array) == 3)
    {
        background.r = ini_data_get_as_int(ini_data_get_from_array(background_array, 0).result).result;
        background.g = ini_data_get_as_int(ini_data_get_from_array(background_array, 1).result).result;
        background.b = ini_data_get_as_int(ini_data_get_from_array(background_array, 2).result).result;
    }
    else {
        background.r = 200;
        background.g = 200;
        background.b = 200;
    }

    fprintf(stdout, "[CHP8] SDL Foreground color: (%d, %d, %d).\n", foreground.r, foreground.g, foreground.b);
    fprintf(stdout, "[CHP8] SDL Background color: (%d, %d, %d).\n", background.r, background.g, background.b);

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

        if(!threaded)
            cchip8_step(self, 60);

        SDL_SetRenderDrawColor(renderer, background.r, background.g, background.b, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, foreground.r, foreground.g, foreground.b, 255);
        
        if(threaded)
            SDL_LockRWLockForReading(self->display_lock);
        for(uint8_t x = 0; x < 64; x++)
            for(uint8_t y = 0; y < 32; y++)
                if(bitset_get(&self->display_memory, x + y * 64))
                    SDL_RenderPoint(renderer, x, y);
        if(threaded)
            SDL_UnlockRWLock(self->display_lock);
        SDL_RenderPresent(renderer);
        SDL_DelayNS(SDL_NS_PER_SECOND / 60);
    }

    if(threaded)
    {
        self->cpu.interpreter.running = false;
        SDL_WaitThread(cpu_thread, NULL); 
    }

    cchip8_free(self);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static maybe_t cchip8_init_from_ini(cchip8_context_t* self, ini_file_t* config)
{
    maybe_t result = {};
    cchip8_init(self);
    
    // Speed parsing.
    ini_section_t* const core_section = ini_file_get_section(config, "chip8.core").result;
    self->speed = RESULT_GET_OR(ini_data_get_as_int(ini_section_get_data(core_section, "speed").result), 600);
    fprintf(stdout, "[CHP8] Speed: %d ips.\n", self->speed);

    // Input type parsing.
    ini_string_result_t const input_result = ini_data_get_as_string(ini_section_get_data(core_section, "input").result);
    if(!IS_OK(input_result))
    {
        cchip8_free(self);
        result.ok = false;
        result.error = "Input type was not specified in the INI file!";
        return result;
    }
    char* const input = RESULT_GET_VALUE(input_result);
    if(strcmp(input, "sdl") == 0)
    {
        fprintf(stdout, "[CHP8] Input: SDL.\n");
        self->state.get_key_status = cchip8_sdl_get_key_status;
    }
    else if(strcmp(input, "none") == 0)
    {
        fprintf(stdout, "[CHP8] Input: none.\n");
        self->state.get_key_status = cchip8_none_get_key_status;        
    }
    else {
        result.ok = false;
        result.error = "Invalid input type specified in the INI file!";
        return result;
    }

    // Load the ROM.
    ini_string_result_t const rom_path_result = ini_data_get_as_string(ini_section_get_data(core_section, "path").result);
    if(!IS_OK(rom_path_result))
    {
        cchip8_free(self);
        result.ok = false;
        result.error = "CHIP8 ROM path not specified in INI file!";
        return result;
    }
    char* const path = RESULT_GET_VALUE(rom_path_result);
    FILE* rom = fopen(path, "rb");
    if(rom == NULL)
    {
        cchip8_free(self);
        result.ok = false;
        result.error = "Could not load specified ROM file!";
        return result;
    }
    fread((char*)self->memory + self->state.pc, sizeof(uint8_t), 0x10000 - self->state.pc, rom);
    fclose(rom);
    fprintf(stdout, "[CHP8] Using ROM at path: %s\n", path);

    result.ok = true;
    return result;
}

maybe_t cchip8_run_from_ini(ini_file_t* config)
{
    fprintf(stdout, "[INFO] Running CHIP8 emulator from INI file.\n");
    maybe_t result = {};
    cchip8_context_t emulator;
    cchip8_init_from_ini(&emulator, config);
    
    // Output type parsing and running.
    ini_section_t* const core_section = ini_file_get_section(config, "chip8.core").result;
    ini_string_result_t const output_result = ini_data_get_as_string(ini_section_get_data(core_section, "output").result);
    if(!IS_OK(output_result))
    {
        cchip8_free(&emulator);
        result.ok = false;
        result.error = "Output type was not specified in the INI file!";
        return result;
    }
    char* const output = RESULT_GET_VALUE(output_result);
    if(strcmp(output, "sdl") == 0)
        cchip8_run_sdl(&emulator, config);
    else if(strcmp(output, "none") == 0)
        cchip8_run_none(&emulator, config);
    else {
        result.ok = false;
        result.error = "Invalid output type specified in the INI file!";
        return result;
    }
    result.ok = true;
    return result;
}
