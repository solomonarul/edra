#include <auxum/std.h>
#include <auxum/file/ini.h>
#include <drivers/chip8.h>

// Error handler.
static void show_error(char* const error)
{
    fprintf(stderr, "[EROR] %s\n", error);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", error, NULL);
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
    UNUSED(argc);
    UNUSED(argv);
    cchip8_context_t self;
    cchip8_init(&self);
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
    SDL_memcpy(self.memory + 0x200, norom, 62 * sizeof(uint8_t));

    // Init SDL subsystem.
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        cchip8_free(&self);
        show_error("Could not initialize SDL3!");
        return -1;
    }
    SDL_Window* window = SDL_CreateWindow("cchip8", 640, 320, SDL_WINDOW_RESIZABLE);
    if(window == NULL)
    {
        cchip8_free(&self);
        show_error("Could not create SDL3 window!");
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if(renderer == NULL)
    {
        cchip8_free(&self);
        show_error("Could not create SDL3 renderer!");
        SDL_DestroyWindow(window);
        return -1;
    }
    SDL_SetRenderLogicalPresentation(renderer, 64, 32, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Create CPU thread.
    SDL_Thread* cpu_thread = SDL_CreateThread(cpu_thread_function, "c8 cpu thr", &self);
    if(cpu_thread == NULL)
    {
        cchip8_free(&self);
        show_error("Could not create CHIP8 emulator thread!");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 0;
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
        
        SDL_LockRWLockForReading(self.display_lock);
        for(uint8_t x = 0; x < 64; x++)
            for(uint8_t y = 0; y < 32; y++)
                if(bitset_get(&self.display_memory, x + y * 64))
                    SDL_RenderPoint(renderer, x, y);
        SDL_UnlockRWLock(self.display_lock);
        SDL_RenderPresent(renderer);
        SDL_DelayNS(SDL_NS_PER_SECOND / 60);
    }

    self.cpu.interpreter.running = false;
    SDL_WaitThread(cpu_thread, NULL); 

    cchip8_free(&self);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
