#include "state.h"

#include <string.h>

void chip8_state_init(chip8_state_t* self)
{
    self->pc = 0x200;
    memset(self->v, 0x00, sizeof(uint8_t) * 0x10);
    self->read_b = NULL; self->read_w = NULL; self->write_b = NULL; self->sp = 0x00;
    self->dt = 0; self->st = 0; self->last_key = 0x10; self->get_random = NULL;
    self->draw_sprite = NULL; self->clear_screen = NULL; self->get_key_status = NULL;
    self->draw_flag = false;
    self->aux_arg = NULL;
}

void chip8_state_log(chip8_state_t* self, FILE* file)
{
    fprintf(file, "[CHP8] Current CHIP8 state:\n");
    fprintf(file, "| PC: %04X |  I: %04X |\n", self->pc, self->i);
    for(uint8_t base = 0x00; base <= 0x8; base += 0x8)
    {
        for(uint8_t index = base; index < base + 0x8; index++)
            fprintf(file, "| V[%01X]: %02X ", index, self->v[index]);
        fprintf(file, "|\n");
    }
}
