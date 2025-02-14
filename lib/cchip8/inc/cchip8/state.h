#pragma once
#ifndef CHIP8_STATE_H
#define CHIP8_STATE_H

#include <stdio.h>
#include <stdint.h>

typedef uint8_t(*chip8_read_b_t)(uint16_t);
typedef uint16_t(*chip8_read_w_t)(uint16_t);
typedef void(*chip8_write_b_t)(uint16_t, uint8_t);
typedef bool(*chip8_draw_sprite_t)(uint16_t, uint8_t, uint8_t, uint8_t);
typedef void(*chip8_clear_t)();

struct chip8_state
{
    uint16_t pc, i;
    uint8_t v[0x10];
    chip8_read_b_t read_b;
    chip8_read_w_t read_w;
    chip8_write_b_t write_b;
    chip8_draw_sprite_t draw_sprite;
    chip8_clear_t clear_screen;
};
typedef struct chip8_state chip8_state_t;

void chip8_state_init(chip8_state_t* self);
void chip8_state_log(chip8_state_t* self, FILE* file);

#endif
