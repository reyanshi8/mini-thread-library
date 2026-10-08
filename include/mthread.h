#ifndef MTHREAD_H
#define MTHREAD_H

#include <stddef.h>

/*
 * Mini User-Level Thread Library
 * Public API
 */

/* =========================================================================
 * Thread States & Types
 * ========================================================================= */

typedef enum {
    MT_READY = 0,
    MT_RUNNING = 1,
    MT_BLOCKED = 2,
    MT_TERMINATED = 3
} mt_thread_state_t;

/* =========================================================================
 * Thread Management (Person 1)
 * ========================================================================= */

/* Initialize the thread library (registers main thread as thread 0) */
void mt_init(void);

/* Create a new thread with function pointer and argument */
int mt_create(void (*function)(void *), void *arg);

/* Give up the CPU voluntarily to next ready thread */
void mt_yield(void);

/* Terminate the currently running thread */
void mt_exit(void);

/* Wait for a specific thread to terminate */
int mt_join(int thread_id);

/* Get the ID of the currently running thread */
int mt_self(void);

/* Alias for mt_self (for scheduler compatibility) */
int mt_get_current(void);

/* Get the current state of a thread */
mt_thread_state_t mt_get_state(int thread_id);

/* Convert a thread state to human-readable string */
const char *mt_state_to_str(mt_thread_state_t state);

/* Check if a thread ID exists and is valid */
int mt_is_valid(int thread_id);

/* Get the total number of threads created so far */
int mt_get_thread_count(void);

/* Transition a thread to MT_BLOCKED. If thread_id is current, switches CPU */
int mt_block(int thread_id);

/* Transition a thread from MT_BLOCKED to MT_READY and re-enqueue in ready queue */
int mt_unblock(int thread_id);

/* Trigger scheduling (cooperative yield hook for scheduler) */
void mt_schedule(void);

/* =========================================================================
 * Ready Queue (Person 1)
 * ========================================================================= */

/* Initialize the ready queue */
void mt_queue_init(void);

/* Add a thread to the ready queue (rejects duplicates and invalid IDs) */
int mt_queue_push(int thread_id);

/* Remove and return the next thread from the ready queue (-1 if empty) */
int mt_queue_pop(void);

/* Check whether the ready queue is empty */
int mt_queue_is_empty(void);

/* Return number of elements currently in the ready queue */
int mt_queue_size(void);

/* Check if a specific thread ID is currently in the ready queue */
int mt_queue_contains(int thread_id);

/* Remove a specific thread ID from the ready queue */
int mt_queue_remove(int thread_id);

/* =========================================================================
 * Mutex (Person 3)
 * ========================================================================= */

typedef struct {
    int locked;
    int owner;
} mt_mutex_t;

void mt_mutex_init(mt_mutex_t *mutex);
void mt_mutex_lock(mt_mutex_t *mutex);
void mt_mutex_unlock(mt_mutex_t *mutex);

/* =========================================================================
 * Semaphore (Person 3)
 * ========================================================================= */

typedef struct {
    int value;
} mt_sem_t;

void mt_sem_init(mt_sem_t *sem, int value);
void mt_sem_wait(mt_sem_t *sem);
void mt_sem_post(mt_sem_t *sem);

#endif /* MTHREAD_H */