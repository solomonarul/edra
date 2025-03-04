#include <auxum/std.h>
#include <auxum/file/ini.h>
#include <drivers/chip8.h>

static void show_error(char* const error)
{
    fprintf(stderr, "[EROR] %s\n", error);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", error, NULL);
    exit(-1);
}

static void show_sdl_error(char* const error)
{
    fprintf(stderr, "[EROR] %s(SDL Error: %s)\n", error, SDL_GetError());
    int first_length = strlen(error);
    int second_length = strlen(SDL_GetError());
    char result[first_length + second_length + 1];
    string_nconcat(result, error, first_length, (char* const)SDL_GetError(), second_length);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", result, NULL);
    exit(-1);
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

int main(int argc, char* argv[])
{
    UNUSED(show_error);
    UNUSED(argc);
    UNUSED(argv);

    // Init SDL subsystem.
    if(!SDL_Init(SDL_INIT_VIDEO))
        show_sdl_error("Could not initialize SDL3! ");
#ifdef BUILD_TYPE_VITA
    SDL_Window* window = SDL_CreateWindow("cchip8", 960, 544, 0);
#else
    SDL_Window* window = SDL_CreateWindow("cchip8", 640, 320, SDL_WINDOW_RESIZABLE);
#endif
    if(window == NULL)
        show_sdl_error("Could not create SDL3 window! ");

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if(renderer == NULL)
    {
        SDL_DestroyWindow(window);
        show_sdl_error("Could not create SDL3 renderer! ");
    }
    SDL_SetRenderVSync(renderer, 1);

    cchip8_context_t self;
    cchip8_init(&self);
    /*FILE* rom = fopen("./roms/chip8/hires/Hires Particle Demo [zeroZshadow, 2008].ch8", "rb");
    if(rom == NULL)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        show_error("CHIP8 ROM file does not exist!");
    }
    fread(self.memory + 0x200, sizeof(uint8_t), 0x1000 - self.state.pc, rom);*/
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
    memcpy(self.memory + 0x200, norom, 62 * sizeof(uint8_t));

    // Create CPU thread.
    SDL_Thread* cpu_thread = SDL_CreateThread(cpu_thread_function, "c8 cpu thr", &self);
    if(cpu_thread == NULL)
    {
        cchip8_free(&self);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        show_sdl_error("Could not create CHIP8 emulator thread!");
    }

    // Main loop.
    SDL_Event event;
    bool app_running = true;
    while(app_running)  // No need to sync here, running one frame more than the CPU can't hurt.
    {
        SDL_SetRenderLogicalPresentation(renderer, self.display_width, self.display_height, SDL_LOGICAL_PRESENTATION_LETTERBOX);

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
        
        SDL_LockRWLockForReading(self.display_lock);
        for(uint8_t x = 0; x < self.display_width; x++)
            for(uint8_t y = 0; y < self.display_height; y++)
                if(bitset_get(&self.display_memory, x + y * self.display_width))
                    SDL_RenderPoint(renderer, x, y);
        SDL_UnlockRWLock(self.display_lock);
        SDL_RenderPresent(renderer);
    }

    self.cpu.interpreter.running = false;
    SDL_WaitThread(cpu_thread, NULL); 

    cchip8_free(&self);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
