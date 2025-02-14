#include "cpu/interpreter.h"

#include <assert.h>

void chip8_interpreter_init(chip8_interpreter_t* self)
{
    self->state = NULL;
    self->running = false;
}

void chip8_intepreter_log_error(chip8_interpreter_t* self, uint16_t opcode)
{
    self->running = false;
    fprintf(stderr, "[EROR]: Unknown opcode %04X found at PC %04X!\n", opcode, self->state->pc);
    chip8_state_log(self->state, stderr);
}

void chip8_interpreter_step(chip8_interpreter_t* self)
{
    chip8_state_t* const state = self->state;

    assert(state != NULL);

    const uint16_t opcode = state->read_w(state->pc);
    #define NOOO (opcode & 0xF000)
    #define OOON (opcode & 0x000F)
    #define OONN (opcode & 0x00FF)
    #define ONNN (opcode & 0x0FFF)
    #define X ((opcode & 0x0F00) >> 8)
    #define Y ((opcode & 0x00F0) >> 4)
    switch(NOOO)
    {
    case 0x0000:
        switch(OONN)
        {
        // CLS
        case 0xE0:
            assert(state->clear_screen != NULL);
            state->clear_screen();
            state->pc += 2;
            break;

        default:
            chip8_intepreter_log_error(self, opcode);
            break;
        }
        break;

    // JMP NNN
    case 0x1000:
        if(state->pc == ONNN)
        {
            self->running = false;
            fprintf(stderr, "[WARN]: Infinite jump found at PC %04X!\n", state->pc);
        }
        state->pc = ONNN;
        break;

    // LD Vx, NN
    case 0x6000:
        state->v[X] = OONN;
        state->pc += 2;
        break;

    // ADD Vx, NN
    case 0x7000:
        state->v[X] += OONN;
        state->pc += 2;
        break;

    // LD I, NNN
    case 0xA000:
        state->i = ONNN;
        state->pc += 2;
        break;

    // DRW, X, Y, N
    case 0xD000:
        assert(state->draw_sprite != NULL);
        state->v[0xF] = state->draw_sprite(state->i, state->v[X], state->v[Y], OOON);
        state->pc += 2;
        break;

    default:
        chip8_intepreter_log_error(self, opcode);
        break;
    }
}
