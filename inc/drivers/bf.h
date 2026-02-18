#pragma once
#ifndef CBF_DRIVER_H
#define CBF_DRIVER_H

#include <stdio.h>
#include <cbf/state.h>
#include <cbf/cpu/interpreter.h>

#ifndef BF_MAX_MEM_SIZE
#define BF_MAX_MEM_SIZE 0x10000
#endif

typedef enum bf_run_mode {
    BF_RUN_INTERPRETER = 0,
} bf_run_mode_t;

typedef struct cbf_context {
    bf_state_t state;
    union {
        bf_interpreter_t interpreter;
    } cpu;
    bf_run_mode_t cpu_run_mode;
    uint8_t memory[BF_MAX_MEM_SIZE];
} cbf_context_t;

void cbf_init(cbf_context_t* self, bf_run_mode_t run_mode);
size_t cbf_load(cbf_context_t* self, char* const rom);
size_t cbf_read(cbf_context_t* self, FILE* file);
bool cbf_is_running(cbf_context_t* self);
void cbf_step(cbf_context_t* self);
void cbf_free(cbf_context_t* self);

#endif
