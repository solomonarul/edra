#include "drivers/chip8.h"
#include "system/input.h"
#include "system/state.h"
#include <SDL3/SDL_gamepad.h>
#include <auxum/std.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

static uint8_t memory_read_b(void* arg, uint16_t address)
{
    cchip8_context_t* const self = arg;
    return self->memory[address];
}

static uint16_t memory_read_w(void* arg, uint16_t address)
{
    cchip8_context_t* const self = arg;
    return (self->memory[address] << 8) | self->memory[address + 1]; 
}

static void memory_write_b(void* arg, uint16_t address, uint8_t value)
{
    cchip8_context_t* const self = arg;
    self->memory[address] = value;
}

static void clear_screen(void* arg)
{
    cchip8_context_t* const self = arg;
    if(self->threaded)
        SDL_LockRWLockForWriting(self->display_lock);

    bitset_clear(&self->display_memory);

    if(self->threaded)
        SDL_UnlockRWLock(self->display_lock);
}

static void resize_screen(void* arg, uint8_t width, uint8_t height)
{
    cchip8_context_t* const self = arg;
    self->state.display_width = width;
    self->state.display_height = height;
    bitset_resize(&self->display_memory, width * height);
}

static void scroll_screen(void* arg, uint8_t amount, chip8_scroll_direction_t direction)
{
    cchip8_context_t* const self = arg;
    UNUSED(amount);
    if(self->threaded)
        SDL_LockRWLockForWriting(self->display_lock);
    switch(direction)
    {
    case CHIP8_SCROLL_DOWN:
        for(uint8_t index = self->state.display_height - 1; index >= amount; index--)
        {
            for(uint8_t pixel = 0; pixel < self->state.display_width; pixel++)
            {
                const int position = pixel + index * self->state.display_width;
                const int otherPosition = pixel + (index - amount) * self->state.display_width;
                bool value = bitset_get(&self->display_memory, otherPosition);
                if(value)
                    bitset_set(&self->display_memory, position);
                else
                    bitset_reset(&self->display_memory, position);
                bitset_reset(&self->display_memory, otherPosition);
            }
        }
        break;
    case CHIP8_SCROLL_LEFT:
        for(uint8_t index = 0; index < self->state.display_height; index++)
        {
            for(uint8_t pixel = 0; pixel < self->state.display_width - amount; pixel++)
            {
                const int position = pixel + index * self->state.display_width;
                const int otherPosition = (pixel + amount) + index * self->state.display_width;
                bool value = bitset_get(&self->display_memory, otherPosition);
                if(value)
                    bitset_set(&self->display_memory, position);
                else
                    bitset_reset(&self->display_memory, position);
                bitset_reset(&self->display_memory, otherPosition);
            }
        }
        break;
    case CHIP8_SCROLL_RIGHT:
        for(uint8_t index = 0; index < self->state.display_height; index++)
        {
            for(uint8_t pixel = self->state.display_width - 1; pixel >= amount; pixel--)
            {
                const int position = pixel + index * self->state.display_width;
                const int otherPosition = (pixel - amount) + index * self->state.display_width;
                bool value = bitset_get(&self->display_memory, otherPosition);
                if(value)
                    bitset_set(&self->display_memory, position);
                else
                    bitset_reset(&self->display_memory, position);
                bitset_reset(&self->display_memory, otherPosition);
            }
        }
        break;
    }
    if(self->threaded)
        SDL_UnlockRWLock(self->display_lock);
}

bool draw_sprite(void* arg, uint16_t address, uint8_t x, uint8_t y, uint8_t n)
{
    cchip8_context_t* const self = arg;
    x %= self->state.display_width;
    y %= self->state.display_height;
    bool collision = false;
    if(self->threaded)
        SDL_LockRWLockForWriting(self->display_lock);
    if(self->state.mode == CHIP8_MODE_SCHIP_MODERN && n == 0)   // Wide drawing.
    {
        n = 0x10;
        for(uint16_t index = address; index < address + n * 2; index += 2)
        {
            const uint16_t word = (self->memory[index] << 8) | self->memory[index + 1];
            for(uint8_t bit = 0; bit < 16; bit++)
            {
                const uint8_t new_x = x + (15 - bit);
                const uint8_t new_y = y + (index - address) / 2;
                if(new_x >= self->state.display_width || new_y >= self->state.display_height) continue;
                const int position = new_x + new_y * self->state.display_width;
                const bool pixel = (word & (1 << bit)) != 0;
                if(pixel == true && bitset_get(&self->display_memory, position) == pixel) collision = true;
                bitset_xor(&self->display_memory, position, pixel);
            }
        }
    }
    else {
        for(uint16_t index = address; index < address + n; index++)
        {
            const uint8_t byte = self->memory[index];
            for(uint8_t bit = 0; bit < 8; bit++)
            {
                const uint8_t new_x = x + (7 - bit);
                const uint8_t new_y = y + (index - address);
                if(new_x >= self->state.display_width || new_y >= self->state.display_height) continue;
                const int position = new_x + new_y * self->state.display_width;
                const bool pixel = (byte & (1 << bit)) != 0;
                if(pixel == true && bitset_get(&self->display_memory, position) == pixel) collision = true;
                bitset_xor(&self->display_memory, position, pixel);
            }
        }
    }
    if(self->threaded)
        SDL_UnlockRWLock(self->display_lock);
    return collision;
}

bool no_key(void* arg, uint8_t index)
{
    UNUSED(arg); UNUSED(index);
    return false;
}

uint8_t get_random(void* arg)
{
    UNUSED(arg);
    return rand();
}

void cchip8_free(cchip8_context_t* self)
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
    self->state.resize = resize_screen;
    self->state.get_key_status = no_key;
    self->state.scroll = scroll_screen;
    self->state.aux_arg = self;
    
    self->state.display_width = 64;
    self->state.display_height = 32;
    bitset_init(&self->display_memory, 64 * 32);
    self->display_lock = SDL_CreateRWLock();

    // Setup CPU.
    chip8_interpreter_init(&self->cpu.interpreter);
    self->cpu.interpreter.running = true;
    self->cpu.interpreter.state = &self->state;
    self->speed = -1;
    self->threaded = false;
}

void cchip8_load_default_font_hires(cchip8_context_t* self)
{
    static const uint8_t HIRES_FONTSET_SIZE = 80;
    static const uint16_t HIRES_FONTSET[80] = {
        0xC67C, 0xDECE, 0xF6D6, 0xC6E6, 0x007C, // 0
        0x3010, 0x30F0, 0x3030, 0x3030, 0x00FC, // 1
        0xCC78, 0x0CCC, 0x3018, 0xCC60, 0x00FC, // 2
        0xCC78, 0x0C0C, 0x0C38, 0xCC0C, 0x0078, // 3
        0x1C0C, 0x6C3C, 0xFECC, 0x0C0C, 0x001E, // 4
        0xC0FC, 0xC0C0, 0x0CF8, 0xCC0C, 0x0078, // 5
        0x6038, 0xC0C0, 0xCCF8, 0xCCCC, 0x0078, // 6
        0xC6FE, 0x06C6, 0x180C, 0x3030, 0x0030, // 7
        0xCC78, 0xECCC, 0xDC78, 0xCCCC, 0x0078, // 8
        0xC67C, 0xC6C6, 0x0C7E, 0x3018, 0x0070, // 9
        0x7830, 0xCCCC, 0xFCCC, 0xCCCC, 0x00CC, // A
        0x66FC, 0x6666, 0x667C, 0x6666, 0x00FC, // B
        0x663C, 0xC0C6, 0xC0C0, 0x66C6, 0x003C, // C
        0x6CF8, 0x6666, 0x6666, 0x6C66, 0x00F8, // D
        0x62FE, 0x6460, 0x647C, 0x6260, 0x00FE, // E
        0x66FE, 0x6462, 0x647C, 0x6060, 0x00F0 // F
    }; // This should be embeded like this because we can't really handle the endianness right otherwise.
    memcpy((char*)(self->memory + self->state.hires_font_address), HIRES_FONTSET, sizeof(uint8_t) * HIRES_FONTSET_SIZE);
}

void cchip8_load_default_font(cchip8_context_t* self)
{
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
    };
    memcpy((char*)(self->memory + self->state.lowres_font_address), FONTSET, sizeof(uint8_t) * FONTSET_SIZE);
}

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

void cchip8_draw_sdl_gpu(cchip8_context_t* self, app_window_t* window, SDL_GPUCommandBuffer* commands, SDL_GPUTexture* swapchainTexture)
{
    if (self->threaded)
        SDL_LockRWLockForReading(self->display_lock);

    const uint32_t w = self->state.display_width;
    const uint32_t h = self->state.display_height;

    static SDL_GPUTransferBuffer* display_buffer = NULL;
    display_buffer = SDL_CreateGPUTransferBuffer(window->gpu, &(SDL_GPUTransferBufferCreateInfo) {
        .size = w * h,
        .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ
    });
    void* mapped_display_data = SDL_MapGPUTransferBuffer(window->gpu, display_buffer, false);
    for(size_t y = 0; y < h; y++)
        for(size_t x = 0; x < w; x++)
            ((uint8_t*)mapped_display_data)[y * w + x] = bitset_get(&self->display_memory, y * w + x) ? 255 : 0;
    SDL_UnmapGPUTransferBuffer(window->gpu, display_buffer);

    if (self->threaded)
        SDL_UnlockRWLock(self->display_lock);

    static SDL_GPUTexture* display_texture = NULL;
    display_texture = SDL_CreateGPUTexture(window->gpu, &(SDL_GPUTextureCreateInfo){
        .type                   = SDL_GPU_TEXTURETYPE_2D,
        .format                 = SDL_GPU_TEXTUREFORMAT_R8_UNORM,
        .width                  = w,
        .height                 = h,
        .layer_count_or_depth   = 1,
        .num_levels             = 1,
        .usage                  = SDL_GPU_TEXTUREUSAGE_SAMPLER
    });

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commands);
	SDL_UploadToGPUTexture(
		copyPass,
		&(SDL_GPUTextureTransferInfo) {
			.transfer_buffer = display_buffer,
			.offset = 0,
		},
		&(SDL_GPUTextureRegion){
			.texture = display_texture,
			.w = w,
			.h = h,
			.d = 1
		},
		false
	);
    SDL_EndGPUCopyPass(copyPass);
    SDL_ReleaseGPUTransferBuffer(window->gpu, display_buffer);

    SDL_GPUColorTargetInfo colorTargetInfo = {0};
    colorTargetInfo.texture = swapchainTexture;
    colorTargetInfo.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f};
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commands, &colorTargetInfo, 1, NULL);
    SDL_EndGPURenderPass(renderPass);

    SDL_GPUBlitInfo blit = {0};
    blit.source.texture = display_texture;
    blit.source.x = 0.0f;
    blit.source.y = 0.0f;
    blit.source.w = (float)w;
    blit.source.h = (float)h;
    blit.destination.texture = swapchainTexture;
    blit.destination.x = 0.0f;
    blit.destination.y = 0.0f;
    blit.destination.w = w * 10.0f; // scaling factor
    blit.destination.h = h * 10.0f;
    blit.filter = SDL_GPU_FILTER_NEAREST;

    SDL_BlitGPUTexture(commands, &blit);
    SDL_ReleaseGPUTexture(window->gpu, display_texture);
}

bool cchip8_get_sdl_key_status(void* arg, uint8_t key)
{
    cchip8_context_t* const self = arg;
    if(!self->input || key >= 0x10)
        return false;

    static SDL_Scancode keys[0x10] = {
        SDL_SCANCODE_X,
        SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
        SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E,
        SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D,
        SDL_SCANCODE_Z,
        SDL_SCANCODE_C, SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F,
        SDL_SCANCODE_V,
    };

    app_input_gamepad_state_t* first_gamepad = app_input_state_get_gamepad(self->input, 0);
    static SDL_GamepadButton gp_buttons[0x10] = {
        SDL_GAMEPAD_BUTTON_COUNT,
        SDL_GAMEPAD_BUTTON_COUNT, SDL_GAMEPAD_BUTTON_COUNT, SDL_GAMEPAD_BUTTON_COUNT,
        SDL_GAMEPAD_BUTTON_SOUTH, SDL_GAMEPAD_BUTTON_DPAD_LEFT, SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
        SDL_GAMEPAD_BUTTON_DPAD_DOWN, SDL_GAMEPAD_BUTTON_COUNT, SDL_GAMEPAD_BUTTON_COUNT,
        SDL_GAMEPAD_BUTTON_COUNT,
        SDL_GAMEPAD_BUTTON_COUNT, SDL_GAMEPAD_BUTTON_COUNT, SDL_GAMEPAD_BUTTON_COUNT, SDL_GAMEPAD_BUTTON_COUNT,
        SDL_GAMEPAD_BUTTON_COUNT
    };
    return app_input_state_key_down(self->input, keys[key]) || (first_gamepad && app_input_state_gamepad_button_down(first_gamepad, gp_buttons[key])); 
}

int cchip8_cpu_thread_function(void* args)
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
    return 0;
}

void cchip8_load_default_rom(cchip8_context_t* self)
{
    static size_t NOROM_SIZE = 62;
    uint8_t NOROM[62] =
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
    memcpy(self->memory + 0x200, NOROM, NOROM_SIZE * sizeof(uint8_t));
}
