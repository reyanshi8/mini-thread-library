#define _XOPEN_SOURCE 700

#include "mthread.h"
#include "timer.h"
#include "sched.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

static struct itimerval timer_val;
static sigset_t timer_sigmask;
static int timer_active = 0;

/* Signal handler for SIGALRM */
static void timer_sigalrm_handler(int signum)
{
    (void)signum;
    sched_tick(signum);
}

/* Initialize timer preemption */
void timer_init(long interval_us)
{
    struct sigaction sa;

    sigemptyset(&timer_sigmask);
    sigaddset(&timer_sigmask, SIGALRM);

    sa.sa_handler = timer_sigalrm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction");
        return;
    }

    if (interval_us <= 0) {
        interval_us = DEFAULT_QUANTUM_US;
    }

    timer_val.it_interval.tv_sec = interval_us / 1000000;
    timer_val.it_interval.tv_usec = interval_us % 1000000;
    timer_val.it_value = timer_val.it_interval;

    timer_active = 1;
    timer_start();
}

/* Start or reset the interval timer */
void timer_start(void)
{
    if (timer_active) {
        setitimer(ITIMER_REAL, &timer_val, NULL);
    }
}

/* Stop the interval timer */
void timer_stop(void)
{
    struct itimerval zero_timer;
    zero_timer.it_interval.tv_sec = 0;
    zero_timer.it_interval.tv_usec = 0;
    zero_timer.it_value = zero_timer.it_interval;

    setitimer(ITIMER_REAL, &zero_timer, NULL);
}

/* Block SIGALRM signal during critical sections */
void timer_block_signals(void)
{
    sigprocmask(SIG_BLOCK, &timer_sigmask, NULL);
}

/* Unblock SIGALRM signal after critical sections */
void timer_unblock_signals(void)
{
    sigprocmask(SIG_UNBLOCK, &timer_sigmask, NULL);
}
