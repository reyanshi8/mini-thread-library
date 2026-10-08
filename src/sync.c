#include "mthread.h"
#include <stddef.h>

/* =========================
 * Mutex Implementation
 * =========================
 */

/*
 * Initialize the mutex.
 * locked = 0 indicates unlocked, owner = -1 indicates no owner.
 */
void mt_mutex_init(mt_mutex_t *mutex)
{
    if (mutex == NULL) {
        return;
    }

    mutex->locked = 0;
    mutex->owner = -1;
}

/*
 * Acquire the mutex.
 * If the mutex is already locked, yield execution until it is free.
 */
void mt_mutex_lock(mt_mutex_t *mutex)
{
    if (mutex == NULL) {
        return;
    }

    while (mutex->locked) {
        mt_yield();
    }

    mutex->locked = 1;
    mutex->owner = mt_self();
}

/*
 * Release the mutex.
 * Resets the lock state and owner.
 */
void mt_mutex_unlock(mt_mutex_t *mutex)
{
    if (mutex == NULL) {
        return;
    }

    if (mutex->locked) {
        mutex->locked = 0;
        mutex->owner = -1;
    }
}

/* =========================
 * Semaphore Implementation
 * =========================
 */

/*
 * Initialize counting semaphore with the given initial value.
 */
void mt_sem_init(mt_sem_t *sem, int value)
{
    if (sem == NULL) {
        return;
    }

    sem->value = value;
}

/*
 * Wait (P / Down) operation on semaphore.
 * If value <= 0, yield execution until value becomes positive,
 * then decrement value.
 */
void mt_sem_wait(mt_sem_t *sem)
{
    if (sem == NULL) {
        return;
    }

    while (sem->value <= 0) {
        mt_yield();
    }

    sem->value--;
}

/*
 * Post (V / Up / Signal) operation on semaphore.
 * Increments the semaphore value.
 */
void mt_sem_post(mt_sem_t *sem)
{
    if (sem == NULL) {
        return;
    }

    sem->value++;
}
