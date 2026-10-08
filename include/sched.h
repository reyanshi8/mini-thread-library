#ifndef SCHED_H
#define SCHED_H

#include <stddef.h>

#define DEFAULT_QUANTUM_US 10000 /* 10 ms default time quantum */

/* Initialize Round Robin scheduler */
void sched_init(long quantum_us);

/* Trigger a scheduler tick (called on SIGALRM expiration) */
void sched_tick(int signum);

/* Voluntarily yield the CPU under Round Robin policy */
void sched_yield(void);

/* Set time quantum in microseconds */
void sched_set_quantum(long quantum_us);

/* Get time quantum in microseconds */
long sched_get_quantum(void);

#endif /* SCHED_H */
