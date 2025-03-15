#include <auxum/std.h>
#include <auxum/file/ini.h>
#include <drivers/chip8.h>
#include <stdlib.h>
#include <GL/gl.h>

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

#ifdef BUILD_TYPE_VITA
int _newlib_heap_size_user   = 100 * 1024 * 1024;   // 100MB VITASDK heap.
unsigned int sceLibcHeapSize = 10 * 1024 * 1024;    // 10MB SCELibC heap.
#endif

void cchip8_draw_gl(cchip8_context_t* self, int window_x, int window_y)
{
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_LockRWLockForReading(self->display_lock);
    int size_x = (window_x / self->state.display_width), size_y = (window_y / self->state.display_height);
    for(uint8_t x = 0; x < self->state.display_width; x++)
        for(uint8_t y = 0; y < self->state.display_height; y++)
            if(bitset_get(&self->display_memory, x + y * self->state.display_width))
            {
                glColor3f(1.0f, 1.0f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(x * size_x, y * size_y);
                glVertex2f((x + 1) * size_x, y * size_y);
                glVertex2f((x + 1) * size_x, (y + 1) * size_y);
                glVertex2f(x * size_x, (y + 1) * size_y);
                glEnd();
            }
    SDL_UnlockRWLock(self->display_lock);
    glFlush();
}

int main(int argc, char* argv[])
{
    UNUSED(show_error);
    UNUSED(argc);
    UNUSED(argv);
#ifdef BUILD_TYPE_VITA
    SDL_SetHint(SDL_HINT_VITA_PVR_OPENGL, "true");
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#endif
    // Init SDL subsystem.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    if(!SDL_Init(SDL_INIT_VIDEO))
        show_sdl_error("Could not initialize SDL3 video subsystem! ");
#ifdef BUILD_TYPE_VITA
    SDL_Window* window = SDL_CreateWindow("Edra", 960, 544, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS);
    int window_x = 960, window_y = 544;
    UNUSED(window_x), UNUSED(window_y);
#else
    SDL_Window* window = SDL_CreateWindow("Edra", 640, 320, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    int window_x = 640, window_y = 320;
    UNUSED(window_x), UNUSED(window_y);
#endif
    if(window == NULL)
        show_sdl_error("Could not create SDL3 window! ");

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if(glContext == NULL)
    {
        SDL_DestroyWindow(window);
        show_sdl_error("Could not create SDL3 GL context! ");
    }
    SDL_GL_MakeCurrent(window, glContext);
    /*SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if(renderer == NULL)
    {
        SDL_DestroyWindow(window);
        show_sdl_error("Could not create SDL3 renderer! ");
    }*/

    cchip8_context_t self;
    cchip8_init(&self);
    self.state.get_key_status = cchip8_get_sdl_key_status;
    self.speed = 6000;
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
    memcpy(self.memory + 0x200, norom, 62 * sizeof(uint8_t));

    // Create CPU thread.
    SDL_Thread* cpu_thread = SDL_CreateThread(cchip8_cpu_thread_function, "c8 cpu thr", &self);
    if(cpu_thread == NULL)
    {
        cchip8_free(&self);
        // SDL_DestroyRenderer(renderer);
        SDL_GL_DestroyContext(glContext);
        SDL_DestroyWindow(window);
        show_sdl_error("Could not create CHIP8 emulator thread!");
    }
    self.threaded = true;

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
                window_x = event.window.data1;
                window_y = event.window.data2;
                break;
            }
        }

        // cchip8_draw_sdl(&self, renderer);
        glViewport(0, 0, window_x, window_y);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, window_x * 1.0, window_y * 1.0, 0.0, -1.0, 1.0);  // 2D orthogonal projection
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        cchip8_draw_gl(&self, window_x, window_y);
        SDL_GL_SwapWindow(window);
        // SDL_RenderPresent(renderer);
    }

    self.cpu.interpreter.running = false;
    SDL_WaitThread(cpu_thread, NULL); 

    cchip8_free(&self);
    SDL_GL_DestroyContext(glContext);
    // SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
