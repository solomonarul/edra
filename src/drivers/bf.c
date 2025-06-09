#include "drivers/bf.h"
#include "auxum/data/dynarray.h"
#include "cbf/cpu/jit_lightning.h"
#include "cbf/state.h"

#include <stdlib.h>

#include <auxum/std.h>
#include <auxum/file/utils.h>

uint8_t cbf_in_f(void* data)
{
    UNUSED(data);
    int8_t ch = getchar();
    if (ch != EOF)
        return ch;
    return 0;
}

void cbf_out_f(void* data, uint8_t ch)
{
    UNUSED(data);
    printf("%c", ch);
}

void cbf_store_f(void* data, uint16_t addr, uint8_t val)
{
    cbf_context_t* const self = (cbf_context_t*) data;
    self->memory[addr] = val;
}

uint8_t cbf_load_f(void* data, uint16_t addr)
{
    cbf_context_t* const self = (cbf_context_t*) data;
    return self->memory[addr];
}

void cbf_init(cbf_context_t* self, bf_run_mode_t run_mode)
{
    bf_state_init(&self->state);
    self->cpu_run_mode = run_mode;
    self->state.in = cbf_in_f; self->state.out = cbf_out_f;
    self->state.store = cbf_store_f; self->state.load = cbf_load_f;
    self->state.aux_arg = self;
    switch(self->cpu_run_mode)
    {
    case BF_RUN_INTERPRETER:
        bf_interpreter_init(&self->cpu.interpreter, &self->state);
        break;
    
#ifdef JIT_LIGHTNING
    case BF_RUN_JIT_LIGHTNING:
        bf_jit_lightning_init(&self->cpu.jit_lightning, &self->state);
        self->cpu.jit_lightning.memory = self->memory;
        break;
#endif
    }
}

size_t cbf_load(cbf_context_t* self, char* const rom, bf_optimizations_t optimizations)
{
    switch(self->cpu_run_mode)
    {
    case BF_RUN_INTERPRETER:
        return bf_interpreter_load_program(&self->cpu.interpreter, rom, optimizations);
#ifdef JIT_LIGHTNING
    case BF_RUN_JIT_LIGHTNING:
        return bf_jit_lightning_load_program(&self->cpu.jit_lightning, rom, optimizations);
#endif
    }
    return 0;
}

size_t cbf_read(cbf_context_t* self, FILE* file, bf_optimizations_t optimizations)
{
    char* rom = file_read_all(file);
    size_t result = cbf_load(self, rom, optimizations);
    free(rom);
    return result;
}

void cbf_step(cbf_context_t* self)
{
    switch(self->cpu_run_mode)
    {
    case BF_RUN_INTERPRETER:
        bf_interpreter_step(&self->cpu.interpreter);
        break;
    
#ifdef JIT_LIGHTNING
    case BF_RUN_JIT_LIGHTNING:
        bf_jit_lightning_step(&self->cpu.jit_lightning);
        break;
#endif
    }
}

bool cbf_is_running(cbf_context_t* self)
{
    return self->cpu.interpreter.running;
}

void cbf_free(cbf_context_t* self)
{
    UNUSED(self);
}
