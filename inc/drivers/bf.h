#pragma once
#ifndef CBF_DRIVER_H
#define CBF_DRIVER_H

#include <stdio.h>
#include <cbf/state.h>
#include <cbf/cpu/interpreter.h>
#ifdef JIT_LIGHTNING
#include <cbf/cpu/jit_lightning.h>
#endif

typedef enum bf_run_mode {
    BF_RUN_INTERPRETER = 0,
#ifdef JIT_LIGHTNING
    BF_RUN_JIT_LIGHTNING
#endif
} bf_run_mode_t;

typedef struct cbf_context {
    bf_state_t state;
    union {
    #ifdef JIT_LIGHTNING
        bf_jit_lightning_t jit_lightning;
    #endif
        bf_interpreter_t interpreter;
    } cpu;
    bf_run_mode_t cpu_run_mode;
    uint8_t memory[0x10000];
} cbf_context_t;

void cbf_init(cbf_context_t* self, bf_run_mode_t run_mode);
void cbf_load(cbf_context_t* self, char* const rom, bf_optimizations_t optimizations);
void cbf_read(cbf_context_t* self, FILE* file, bf_optimizations_t optimizations);
bool cbf_is_running(cbf_context_t* self);
void cbf_step(cbf_context_t* self);
void cbf_free(cbf_context_t* self);

#endif
