#pragma once
#ifndef AUXUM_THREAD_H
#define AUXUM_THREAD_H

#include <stdint.h>

/*
 *  Gets the current ammount of nanoseconds passed since a system defined epoch.
 */
uint64_t thread_get_nanos(void);

/*
 *  Uses SDL's sleeping as I assume SDL knows better than me about this lol.
 */
void thread_sleep(uint64_t nsecs);

#endif
