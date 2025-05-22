#include "drivers/bf.h"
#include "auxum/data/dynarray.h"
#include "cbf/state.h"

#include <stdlib.h>
#include <string.h>

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

void cbf_init(cbf_context_t* self)
{
    dynarray_init(&self->state.program, sizeof(bf_instruction_t), 0);
    self->state.in = cbf_in_f; self->state.out = cbf_out_f;
    self->state.store = cbf_store_f; self->state.load = cbf_load_f;
    bf_interpreter_init(&self->cpu.interpreter);
    self->cpu.interpreter.state = &self->state;
    self->state.aux_arg = self;
}

void cbf_load(cbf_context_t* self, char* const rom)
{
    dynarray_free(self->state.program);
    dynarray_init(&self->state.program, sizeof(bf_instruction_t), 0);
    int size = strlen(rom);
    bf_instruction_t element;
    for(int index = 0; index < size; index++)
        switch(rom[index])
        {
        case '+':
            {
                element.op = BF_INSTRUCTION_INC;
                element.arg = 0;
                dynarray_push_back(&self->state.program, &element);
                break;
            }
        case '-':
            {
                element.op = BF_INSTRUCTION_DEC;
                element.arg = 0;
                dynarray_push_back(&self->state.program, &element);
                break;
            }
        case '>':
            {
                element.op = BF_INSTRUCTION_NEXT;
                element.arg = 0;
                dynarray_push_back(&self->state.program, &element);
                break;
            }
        case '<':
            {
                element.op = BF_INSTRUCTION_PREV;
                element.arg = 0;
                dynarray_push_back(&self->state.program, &element);
                break;
            }
        case '[':
            {
                element.op = BF_INSTRUCTION_JUMP_START;
                element.arg = 0;
                dynarray_push_back(&self->state.program, &element);
                break;
            }
        case ']':
            {
                element.op = BF_INSTRUCTION_JUMP_BACK;
                element.arg = 0;
                dynarray_push_back(&self->state.program, &element);
                break;
            }
        case ',':
            {
                element.op = BF_INSTRUCTION_INPUT;
                element.arg = 0;
                dynarray_push_back(&self->state.program, &element);
                break;
            }
        case '.':
            {
                element.op = BF_INSTRUCTION_OUTPUT;
                element.arg = 0;
                dynarray_push_back(&self->state.program, &element);
                break;
            }
        default:
            {
                // Do nothing for unknown characters.
                break;
            }
        }
    element.op = BF_INSTRUCTION_END;
    element.arg = 0;
    dynarray_push_back(&self->state.program, &element);
}

void cbf_read(cbf_context_t* self, FILE* file)
{
    char* rom = file_read_all(file);
    cbf_load(self, rom);
    free(rom);
}

void cbf_free(cbf_context_t* self)
{
    (void)self;
}
