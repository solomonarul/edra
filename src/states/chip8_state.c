#include "states/chip8_state.h"

static void cchip8_app_update(void* self_ref, long dt)
{
    chip8_app_state_t* const self = (chip8_app_state_t*)self_ref;
    UNUSED(self);
    UNUSED(dt);
}

static void cchip8_app_render(void* self_ref, app_window_t* window)
{
    chip8_app_state_t* const self = (chip8_app_state_t*)self_ref;
    cchip8_draw_sdl(&self->emulator, window->renderer);
    SDL_SetRenderLogicalPresentation(
        window->renderer,
        window->size_x, window->size_y,
        SDL_LOGICAL_PRESENTATION_DISABLED
    );
}

static void cchip8_app_pause(void* self_ref)
{
    chip8_app_state_t* const self = (chip8_app_state_t*)self_ref;
    if(self->paused)
        return;
}

static void cchip8_app_unpause(void* self_ref)
{
    chip8_app_state_t* const self = (chip8_app_state_t*)self_ref;
    if(!self->paused)
        return;
}

static void cchip8_app_free(void* self_ref)
{
    chip8_app_state_t* const self = (chip8_app_state_t*)self_ref;
    self->emulator.cpu.interpreter.running = false;
    if(self->thread != NULL)
        SDL_WaitThread(self->thread, NULL); 
    cchip8_free(&self->emulator);
}

void chip8_app_state_init(chip8_app_state_t* self)
{
    cchip8_init(&self->emulator);
    self->emulator.state.get_key_status = cchip8_get_sdl_key_status;
    self->emulator.speed = 600;
    cchip8_load_default_font(&self->emulator);
    cchip8_load_default_rom(&self->emulator);
    self->internal.userdata = self;
    self->internal.free = cchip8_app_free;
    self->internal.unpause = cchip8_app_unpause;
    self->internal.pause = cchip8_app_pause;
    self->internal.render = cchip8_app_render;
    self->internal.update = cchip8_app_update;
}

maybe_t chip8_app_state_load_rom(chip8_app_state_t* self, chip8_run_mode_t run_mode, FILE* rom, bool threaded, bool close_file_on_read)
{
    maybe_t result;
    if(rom == NULL)
    {
        result.ok = false;
        result.error = "CHIP8 ROM doesn't exist!";
        return result;
    }
    self->emulator.state.mode = run_mode;
    if(run_mode == CHIP8_MODE_SCHIP_MODERN)
        cchip8_load_default_font_hires(&self->emulator);
    fread(self->emulator.memory + 0x200, sizeof(uint8_t), 0x1000 - self->emulator.state.pc, rom);
    if(close_file_on_read)
        fclose(rom);

    // Create CPU thread.
    if(threaded)
    {
        self->emulator.threaded = true;
        self->thread = SDL_CreateThread(cchip8_cpu_thread_function, "c8 cpu thr", &self->emulator);
        if(self->thread == NULL)
        {
            cchip8_free(&self->emulator);
            result.ok = false;
            result.error = "Could not create CHIP8 emulator thread!";
            return result;
        }
    }
    result.ok = true;
    return result;
}
