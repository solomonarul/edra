#pragma once
#ifndef CBF_DRIVER_H
#define CBF_DRIVER_H

#include <stdio.h>
#include <cbf/state.h>
#include <cbf/cpu/interpreter.h>

struct cbf_context {
    bf_state_t state;
    union {
        bf_interpreter_t interpreter;
    } cpu;
    uint8_t memory[0x10000];
};
typedef struct cbf_context cbf_context_t;

void cbf_init(cbf_context_t* self);
void cbf_load(cbf_context_t* self, char* const rom);
void cbf_read(cbf_context_t* self, FILE* file);
void cbf_free(cbf_context_t* self);

#endif
