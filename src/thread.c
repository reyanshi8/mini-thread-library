#define _XOPEN_SOURCE 700

#include "mthread.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ucontext.h>

#define MAX_THREADS 16
#define STACK_SIZE  (64 * 1024)

/* Thread Control Block */
typedef struct {
    int id;
    mt_thread_state_t state;

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

/* Deferred stack cleanup for terminated threads to prevent freeing active stack */
static int stack_to_free = -1;

/*
 * Helper to clean up any stack deferred from a previously terminated thread.
 */
static void cleanup_deferred_stack(void)
{
    if (stack_to_free > 0 && stack_to_free < MAX_THREADS) {
        if (threads[stack_to_free].stack != NULL) {
            free(threads[stack_to_free].stack);
            threads[stack_to_free].stack = NULL;
        }
        stack_to_free = -1;
    }
}

/*
 * Function that starts every newly created thread.
 */
static void thread_wrapper(uintptr_t thread_id)
{
    int id = (int)thread_id;

    /* Safe to reap previous terminated thread's stack once we are on this new stack */
    cleanup_deferred_stack();

    current_thread = id;
    threads[id].state = MT_RUNNING;

    threads[id].function(threads[id].arg);

    /*
     * If user thread function completes normally,
     * terminate cleanly.
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

    cleanup_deferred_stack();

    thread_count = 1;
    current_thread = 0;
    stack_to_free = -1;

    for (i = 0; i < MAX_THREADS; i++) {
        threads[i].id = -1;
        threads[i].state = MT_TERMINATED;
        threads[i].stack = NULL;
        threads[i].function = NULL;
        threads[i].arg = NULL;
    }

    /* Thread 0 represents the main program / initial context */
    threads[0].id = 0;
    threads[0].state = MT_RUNNING;

    getcontext(&threads[0].context);
}

/*
 * Create a new user-level thread.
 */
int mt_create(void (*function)(void *), void *arg)
{
    int id;

    if (function == NULL) {
        return -1;
    }

    if (thread_count >= MAX_THREADS) {
        return -1;
    }

    cleanup_deferred_stack();

    id = thread_count;

    threads[id].id = id;
    threads[id].state = MT_READY;
    threads[id].function = function;
    threads[id].arg = arg;

    threads[id].stack = malloc(STACK_SIZE);
    if (threads[id].stack == NULL) {
        threads[id].state = MT_TERMINATED;
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
        threads[id].state = MT_TERMINATED;
        thread_count--;
        return -1;
    }

    return id;
}

/*
 * Voluntarily give up the CPU to next ready thread.
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

    if (threads[previous].state == MT_RUNNING) {
        threads[previous].state = MT_READY;

        if (mt_queue_push(previous) != 0) {
            threads[previous].state = MT_RUNNING;
            return;
        }
    }

    threads[next].state = MT_RUNNING;
    current_thread = next;

    swapcontext(
        &threads[previous].context,
        &threads[next].context
    );

    cleanup_deferred_stack();
}

/*
 * Terminate the currently running thread.
 */
void mt_exit(void)
{
    int next;
    int previous;

    previous = current_thread;

    threads[previous].state = MT_TERMINATED;

    /*
     * Mark stack for deferred deallocation by next thread.
     * Never free stack while actively running on it!
     */
    if (previous != 0) {
        stack_to_free = previous;
    }

    next = mt_queue_pop();

    if (next == -1) {
        /*
         * No other thread in ready queue.
         * Switch back to main thread (thread 0).
         */
        current_thread = 0;
        threads[0].state = MT_RUNNING;

        cleanup_deferred_stack();
        setcontext(&threads[0].context);
        return;
    }

    threads[next].state = MT_RUNNING;
    current_thread = next;

    setcontext(&threads[next].context);
}

/*
 * Wait for a specific thread to terminate.
 */
int mt_join(int thread_id)
{
    if (thread_id < 0 || thread_id >= thread_count) {
        return -1;
    }

    if (thread_id == current_thread) {
        return -1; /* Cannot join self */
    }

    while (threads[thread_id].state != MT_TERMINATED) {
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

/*
 * Alias for mt_self (scheduler helper).
 */
int mt_get_current(void)
{
    return current_thread;
}

/*
 * Return the state of a thread.
 */
mt_thread_state_t mt_get_state(int thread_id)
{
    if (thread_id < 0 || thread_id >= thread_count) {
        return MT_TERMINATED;
    }

    return threads[thread_id].state;
}

/*
 * Convert thread state to human-readable string.
 */
const char *mt_state_to_str(mt_thread_state_t state)
{
    switch (state) {
        case MT_READY:      return "READY";
        case MT_RUNNING:    return "RUNNING";
        case MT_BLOCKED:    return "BLOCKED";
        case MT_TERMINATED: return "TERMINATED";
        default:            return "UNKNOWN";
    }
}

/*
 * Check if a thread ID exists and is within valid bounds.
 */
int mt_is_valid(int thread_id)
{
    return (thread_id >= 0 && thread_id < thread_count);
}

/*
 * Get total number of threads registered in the library.
 */
int mt_get_thread_count(void)
{
    return thread_count;
}

/*
 * Block a thread (used by Person 3 for synchronization: mutex lock, sem wait).
 * If thread_id is the currently running thread, yields CPU immediately.
 */
int mt_block(int thread_id)
{
    if (!mt_is_valid(thread_id)) {
        return -1;
    }

    if (threads[thread_id].state == MT_TERMINATED || threads[thread_id].state == MT_BLOCKED) {
        return -1;
    }

    if (thread_id == current_thread) {
        int previous = current_thread;
        int next = mt_queue_pop();

        if (next == -1) {
            /* If no thread in queue, fall back to main thread (0) if not terminated */
            if (previous != 0 && threads[0].state != MT_TERMINATED) {
                next = 0;
            } else {
                return -1;
            }
        }

        threads[previous].state = MT_BLOCKED;
        threads[next].state = MT_RUNNING;
        current_thread = next;

        swapcontext(
            &threads[previous].context,
            &threads[next].context
        );

        cleanup_deferred_stack();
        return 0;
    } else {
        /* Thread was in ready queue, remove it from ready queue */
        mt_queue_remove(thread_id);
        threads[thread_id].state = MT_BLOCKED;
        return 0;
    }
}

/*
 * Unblock a thread (used by Person 3 for synchronization: mutex unlock, sem post).
 * Moves state from MT_BLOCKED to MT_READY and re-enqueues into ready queue.
 */
int mt_unblock(int thread_id)
{
    if (!mt_is_valid(thread_id)) {
        return -1;
    }

    if (threads[thread_id].state != MT_BLOCKED) {
        return -1;
    }

    threads[thread_id].state = MT_READY;
    return mt_queue_push(thread_id);
}

/*
 * Schedule hook for external modules (e.g. Person 2 preemption handler).
 */
void mt_schedule(void)
{
    mt_yield();
}