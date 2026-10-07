#ifndef TIMER_H
#define TIMER_H

#include <signal.h>

/* Initialize preemption timer with interval in microseconds */
void timer_init(long interval_us);

/* Start or reset the interval timer */
void timer_start(void);

/* Stop the interval timer */
void timer_stop(void);

/* Block SIGALRM to protect critical sections */
void timer_block_signals(void);

/* Unblock SIGALRM after critical sections */
void timer_unblock_signals(void);

#endif /* TIMER_H */
