#include "thread.h"
#include <threads.h>
#ifdef BUILD_TYPE_WINDOWS
#include <windows.h>
#include <timeapi.h>
#endif

void thread_setup(void)
{
#ifdef BUILD_TYPE_WINDOWS
    timeBeginPeriod(1);
#endif
}

unsigned long thread_get_nanos(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (unsigned long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

void thread_spin_sleep(unsigned long nsecs, unsigned long trusted_nsecs)
{
    // Sleep for a multiple of trusted_nsecs.
    if(trusted_nsecs == (unsigned long)-1) trusted_nsecs = 1000000L + 1;    // ~1ms
    long count = nsecs / trusted_nsecs;
#ifdef BUILD_TYPE_WINDOWS
    Sleep(trusted_nsecs / 1000000L * count);
#else
    thrd_sleep(&(struct timespec){.tv_nsec=count * trusted_nsecs}, NULL);
#endif
    // Busy loop for the remainder time.
    long remainder = nsecs % trusted_nsecs;
    long last_nanos = thread_get_nanos();
    long current_nanos = last_nanos;
    while(remainder > current_nanos - last_nanos)
        current_nanos = thread_get_nanos(), remainder -= current_nanos - last_nanos, last_nanos = current_nanos;
}
