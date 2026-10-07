#define _XOPEN_SOURCE 700

#include "mthread.h"
#include "sched.h"
#include "timer.h"

#include <stdio.h>

static long current_quantum_us = DEFAULT_QUANTUM_US;
static int sched_initialized = 0;

/*
 * Initialize the Round Robin scheduler with time quantum in microseconds.
 */
void sched_init(long quantum_us)
{
    if (quantum_us <= 0) {
        quantum_us = DEFAULT_QUANTUM_US;
    }

    current_quantum_us = quantum_us;
    sched_initialized = 1;

    timer_init(current_quantum_us);
}

/*
 * Wrapper for library initialization.
 */
void mt_scheduler_init(long quantum_us)
{
    sched_init(quantum_us);
}

/*
 * Set new time quantum in microseconds.
 */
void sched_set_quantum(long quantum_us)
{
    if (quantum_us <= 0) {
        return;
    }

    current_quantum_us = quantum_us;

    if (sched_initialized) {
        timer_init(current_quantum_us);
    }
}

/*
 * Get current time quantum.
 */
long sched_get_quantum(void)
{
    return current_quantum_us;
}

/*
 * Scheduler tick handler invoked upon SIGALRM expiry.
 * Performs Round Robin preemptive context switch.
 */
void sched_tick(int signum)
{
    (void)signum;

    if (!sched_initialized) {
        return;
    }

    /* Block signals during preemptive yield to prevent nested signal handling */
    timer_block_signals();

    /* Yield CPU to next ready thread in Round Robin queue */
    mt_yield();

    timer_unblock_signals();
}

/*
 * Voluntarily yield under Round Robin policy.
 */
void sched_yield(void)
{
    timer_block_signals();
    mt_yield();
    timer_unblock_signals();
}
