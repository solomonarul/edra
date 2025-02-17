#include "thread.h"
#include <SDL3/SDL.h>

uint64_t thread_get_nanos(void)
{
    return SDL_GetPerformanceCounter() * 1.0 / SDL_GetPerformanceFrequency() * 1e9;
}

void thread_sleep(uint64_t nsecs)
{
    SDL_DelayNS(nsecs); // I trust SDL knows better than me on matters like this, I give up. This uses way less CPU time than my crazy ideas.
}
