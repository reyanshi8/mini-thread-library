#define _XOPEN_SOURCE 700

#include "mthread.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ucontext.h>

#define MAX_THREADS 16
#define STACK_SIZE  (64 * 1024)

/* Thread states */
#define READY       0
#define RUNNING     1
#define TERMINATED  2

/* Thread Control Block */
typedef struct {
    int id;
    int state;

    ucontext_t context;

    void *stack;

    void (*function)(void *);
    void *arg;
} TCB;

/* Thread table */
static TCB threads[MAX_THREADS];

/* Number of threads created */
static int thread_count = 0;

/* Currently running thread */
static int current_thread = -1;


/*
 * Find the next READY thread.
 */



/*
 * Function that starts every newly created thread.
 */
static void thread_wrapper(uintptr_t thread_id)
{
    int id = (int)thread_id;

    current_thread = id;
    threads[id].state = RUNNING;

    threads[id].function(threads[id].arg);

    /*
     * If the user's function returns normally,
     * terminate the thread automatically.
     */
    mt_exit();
}


/*
 * Initialize the thread library.
 */
void mt_init(void)
{
    int i;

    mt_queue_init();

    thread_count = 1;
    current_thread = 0;

    for (i = 0; i < MAX_THREADS; i++) {
        threads[i].id = -1;
        threads[i].state = TERMINATED;
        threads[i].stack = NULL;
        threads[i].function = NULL;
        threads[i].arg = NULL;
    }

    /*
     * Thread 0 represents the main program.
     */
    threads[0].id = 0;
    threads[0].state = RUNNING;

    getcontext(&threads[0].context);
}


/*
 * Create a new user-level thread.
 */
int mt_create(void (*function)(void *), void *arg)
{
    int id;

    if (thread_count >= MAX_THREADS) {
        return -1;
    }

    if (function == NULL) {
        return -1;
    }

    id = thread_count;

    threads[id].id = id;
    threads[id].state = READY;
    threads[id].function = function;
    threads[id].arg = arg;

    threads[id].stack = malloc(STACK_SIZE);

    if (threads[id].stack == NULL) {
        return -1;
    }

    getcontext(&threads[id].context);

    threads[id].context.uc_stack.ss_sp = threads[id].stack;
    threads[id].context.uc_stack.ss_size = STACK_SIZE;
    threads[id].context.uc_link = NULL;

    makecontext(
        &threads[id].context,
        (void (*)(void))thread_wrapper,
        1,
        (uintptr_t)id
    );

    thread_count++;
	
    if (mt_queue_push(id) != 0) {
    free(threads[id].stack);
    threads[id].stack = NULL;
    threads[id].state = TERMINATED;
    return -1;
    }

    return id;
}


/*
 * Voluntarily give up the CPU.
 */
void mt_yield(void)
{
    int next;
    int previous;

    previous = current_thread;
    next = mt_queue_pop();

    if (next == -1) {
        return;
    }

    if (threads[previous].state == RUNNING) {
    threads[previous].state = READY;

    if (mt_queue_push(previous) != 0) {
        threads[previous].state = RUNNING;
        return;
    }
}

threads[next].state = RUNNING;
    current_thread = next;

    swapcontext(
        &threads[previous].context,
        &threads[next].context
    );
}


/*
 * Terminate the current thread.
 */
void mt_exit(void)
{
    int next;
    int previous;

    previous = current_thread;

    threads[previous].state = TERMINATED;

    /*
     * Free the stack of a non-main thread.
     */
    if (previous != 0 && threads[previous].stack != NULL) {
        free(threads[previous].stack);
        threads[previous].stack = NULL;
    }

    next = mt_queue_pop();

    if (next == -1) {
        /*
         * No other thread is ready.
         * Return to the main program.
         */
        current_thread = 0;
        threads[0].state = RUNNING;
        return;
    }

    threads[next].state = RUNNING;
    current_thread = next;

    setcontext(&threads[next].context);
}


/*
 * Wait for a thread to terminate.
 */
int mt_join(int thread_id)
{
    if (thread_id < 0 || thread_id >= thread_count) {
        return -1;
    }

    if (thread_id == current_thread) {
        return -1;
    }

    while (threads[thread_id].state != TERMINATED) {
        mt_yield();
    }

    return 0;
}


/*
 * Return the ID of the currently running thread.
 */
int mt_self(void)
{
    return current_thread;
}