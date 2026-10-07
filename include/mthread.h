#ifndef MTHREAD_H
#define MTHREAD_H

/*
 * Mini User-Level Thread Library
 * Public API
 */

/* Initialize the thread library */
void mt_init(void);

/* Create a new thread */
int mt_create(void (*function)(void *), void *arg);

/* Give up the CPU voluntarily */
void mt_yield(void);

/* Terminate the current thread */
void mt_exit(void);

/* Wait for another thread to finish */
int mt_join(int thread_id);

/* Get the ID of the current thread */
int mt_self(void);

/* =========================
 * Mutex
 * =========================
 */

typedef struct {
    int locked;
    int owner;
} mt_mutex_t;

void mt_mutex_init(mt_mutex_t *mutex);
void mt_mutex_lock(mt_mutex_t *mutex);
void mt_mutex_unlock(mt_mutex_t *mutex);

/* =========================
 * Semaphore
 * =========================
 */

typedef struct {
    int value;
} mt_sem_t;

void mt_sem_init(mt_sem_t *sem, int value);
void mt_sem_wait(mt_sem_t *sem);
void mt_sem_post(mt_sem_t *sem);

#endif