#pragma once
#ifndef APP_TEST_STATE_H
#define APP_TEST_STATE_H

#include "../system/state.h"
#include <auxum/std.h>

typedef struct chip8_pause_app_state {
    bool update_skipped_first_frame;
    app_state_t internal;
} chip8_pause_app_state_t;

void chip8_pause_app_state_init(chip8_pause_app_state_t* self);

#endif
