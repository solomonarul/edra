#pragma once
#ifndef AUXUM_THREAD_H
#define AUXUM_THREAD_H

void thread_setup(void);

/*
 *  Gets the current ammount of nanoseconds passed since a system defined epoch.
 *  On Windows, the ammount it is accurate within 100us.
 */
unsigned long thread_get_nanos(void);

/*
 *  Uses native sleeping for a multiple of trusted_nsecs and then busy waits for the rest for relatively accurate sleeping.
 *  If trusted_nsecs is -1, a default value of 16ms is used, as per Windows thread sleeping accuracy. // TODO: on Windows, timeBeginPeriod(1) to make this be 1ms.
 */
void thread_spin_sleep(unsigned long nsecs, unsigned long trusted_nsecs);

#endif
