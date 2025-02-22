#include "cpu/interpreter.h"

#include <assert.h>

void chip8_interpreter_init(chip8_interpreter_t* self)
{
    self->state = NULL;
    self->running = false;
    self->timer = 0;
}

void chip8_intepreter_log_error(chip8_interpreter_t* self, uint16_t opcode)
{
    self->running = false;
    fprintf(stderr, "[EROR] Unknown opcode %04X found at PC %04X!\n", opcode, self->state->pc);
    chip8_state_log(self->state, stderr);
}

void chip8_interpreter_update_timers(chip8_interpreter_t* self, long dt)
{
    self->timer += dt;
    int remainder = self->timer / 16666666;
    self->timer -= remainder * 16666666;
    self->state->draw_flag = (remainder > 0) ? true : self->state->draw_flag;
    self->state->dt = (self->state->dt < remainder) ? 0 : (self->state->dt - remainder);
    self->state->st = (self->state->st < remainder) ? 0 : (self->state->st - remainder);
}

void chip8_interpreter_step(chip8_interpreter_t* self)
{
    chip8_state_t* const state = self->state;

    assert(state != NULL);
    assert(state->read_w != NULL);
    assert(state->read_b != NULL);
    assert(state->write_b != NULL);
    const uint16_t opcode = state->read_w(state->aux_arg, state->pc);

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
            state->clear_screen(state->aux_arg);
            state->pc += 2;
            break;

        // RET
        case 0xEE:
            assert(state->sp > 0);
            state->pc = state->stack[--state->sp] + 2;
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
            fprintf(stdout, "[CHP8] [WARN] Infinite jump found at PC %04X!\n", state->pc);
        }
        state->pc = ONNN;
        break;

    // CALL NNN
    case 0x2000:
        assert(state->sp < 0xFF);
        state->stack[state->sp++] = state->pc;
        state->pc = ONNN;
        break;
    
    // SE Vx, NN
    case 0x3000:
        state->pc += 2 + (state->v[X] == OONN) * 2;
        break;

    // SNE Vx, NN
    case 0x4000:
        state->pc += 2 + (state->v[X] != OONN) * 2;
        break;

    case 0x5000:
        switch(OOON)
        {
        // SE Vx, Vy
        case 0x0:
            state->pc += 2 + (state->v[X] == state->v[Y]) * 2;
            break;
        default:
            chip8_intepreter_log_error(self, opcode);
            break;
        }
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

    // Arithmetic
    case 0x8000:
        switch(OOON)
        {
        // LD Vx, Vy
        case 0x0:
            state->v[X] = state->v[Y];
            state->pc += 2;
            break;

        // OR Vx, Vy
        case 0x1:
            state->v[X] |= state->v[Y];
            state->v[0xF] = 0x00; // NOTE: only in vanilla CHIP8. Check quirks test,
            state->pc += 2; 
            break;
        
        // AND Vx, Vy
        case 0x2:
            state->v[X] &= state->v[Y];
            state->v[0xF] = 0x00; // NOTE: only in vanilla CHIP8. Check quirks test,
            state->pc += 2;
            break;

        // XOR Vx, Vy
        case 0x3:
            state->v[X] ^= state->v[Y];
            state->v[0xF] = 0x00; // NOTE: only in vanilla CHIP8. Check quirks test,
            state->pc += 2; 
            break;

        // ADD Vx, Vy
        case 0x4:
        {
            const uint8_t flag = (0xFF - state->v[X]) <= state->v[Y];
            state->v[X] += state->v[Y];
            state->v[0xF] = flag;
            state->pc += 2;
            break;
        }

        // SUB Vx, Vy
        case 0x5:
        {
            const uint8_t flag = !(state->v[X] < state->v[Y]);
            state->v[X] -= state->v[Y];
            state->v[0xF] = flag;
            state->pc += 2;
            break;
        }

        // SHR Vx, Vy
        case 0x6:
        {
            const uint8_t flag = state->v[X] & 1;
            state->v[X] = state->v[Y] >> 1;
            state->v[0xF] = flag;
            state->pc += 2; 
            break;
        }

        // SUBN Vx, Vy
        case 0x7:
        {
            const uint8_t flag = !(state->v[Y] < state->v[X]);
            state->v[X] = state->v[Y] - state->v[X];
            state->v[0xF] = flag;
            state->pc += 2;
            break;
        }

        // SHL Vx, Vy
        case 0xE:
        {
            const uint8_t flag = state->v[X] & (1 << 7);
            state->v[X] = state->v[Y] << 1;
            state->v[0xF] = flag >> 7;
            state->pc += 2; 
            break;
        }

        default:
            chip8_intepreter_log_error(self, opcode);
            break;
        }
        break;

    case 0x9000:
        switch(OOON)
        {
        // SNE Vx, Vy
        case 0x0:
            state->pc += 2 + (state->v[X] != state->v[Y]) * 2;
            break;

        default:
            chip8_intepreter_log_error(self, opcode);
            break;
        }
        break;

    // LD I, NNN
    case 0xA000:
        state->i = ONNN;
        state->pc += 2;
        break;

    // JMP V0 + NNN
    case 0xB000:
        state->pc = ONNN + state->v[0];
        break;

    // RND Vx, NN
    case 0xC000:
        state->v[X] = state->get_random(state->aux_arg) & OONN;
        state->pc += 2;
        break;

    // DRW, X, Y, N
    case 0xD000:
        assert(state->draw_sprite != NULL);
        if(state->draw_flag)
        {
            state->v[0xF] = state->draw_sprite(state->aux_arg, state->i, state->v[X], state->v[Y], OOON);
            state->pc += 2;
            state->draw_flag = false;
        }
        break;

    // Keys
    case 0xE000:
        switch(OONN)
        {
        // SKKP Vx
        case 0x9E:
            assert(state->get_key_status);
            state->pc += 2 + (state->get_key_status(state->aux_arg, state->v[X] & 0xF)) * 2;
            break;

        // SKNP Vx
        case 0xA1:
            assert(state->get_key_status);
            state->pc += 2 + !(state->get_key_status(state->aux_arg, state->v[X] & 0xF)) * 2;
            break;

        default:
            chip8_intepreter_log_error(self, opcode);
            break;            
        }
        break;

    // Special
    case 0xF000:
        switch(OONN)
        {
        // LD Vx, DT
        case 0x07:
            state->v[X] = state->dt;
            state->pc += 2;
            break;

        // STKP Vx
        case 0x0A:
            // Nothing pressed yet, check.
            if(state->last_key == 0x10)
            {
                for(uint8_t index = 0; index < 0x10; index++)
                    if(state->get_key_status(state->aux_arg, index))
                    {
                        state->last_key = index;
                        break;
                    }
            }
            // Key has been pressed already, wait for release.
            else if(!state->get_key_status(state->aux_arg, state->last_key))
            {
                state->v[X] = state->last_key;
                state->last_key = 0x10;
                state->pc += 2;
            }
            break;

        // LD DT, Vx
        case 0x15:
            state->dt = state->v[X];
            state->pc += 2;
            break;

        // LD ST, Vx
        case 0x18:
            state->st = state->v[X];
            state->pc += 2;
            break;

        // ADD I, Vx
        case 0x1E:
            state->i += state->v[X];
            state->pc += 2;
            break;

        // SPRT Vx
        case 0x29:
            state->i = 5 * (state->v[X] & 0xF);
            state->pc += 2;
            break;

        // BCD Vx
        case 0x33:
            uint8_t value = state->v[X];
            for(uint8_t index = 0; index < 3; index++)
                state->write_b(state->aux_arg, state->i + 2 - index, value % 10), value /= 10;
            state->pc += 2;
            break;

        // STR Vx
        case 0x55:
            for(uint8_t index = 0; index <= X; index++)
                state->write_b(state->aux_arg, state->i + index, state->v[index]);
            state->i += X + 1;  // NOTE: only in vanilla CHIP8. Check quirks test.
            state->pc += 2;
            break;

        // LDR Vx
        case 0x65:
            for(uint8_t index = 0; index <= X; index++)
                state->v[index] = state->read_b(state->aux_arg, state->i + index);
            state->i += X + 1; // NOTE: only in vanilla CHIP8. Check quirks test,
            state->pc += 2;
            break;        

        default:
            chip8_intepreter_log_error(self, opcode);
            break;
        }
        break;

    default:
        chip8_intepreter_log_error(self, opcode);
        break;
    }
}
