#include <stdio.h>
#include "mthread.h"

#define NUM_ITERATIONS 5

/* Shared resources */
static int counter_no_mutex = 0;
static int counter_with_mutex = 0;
static mt_mutex_t lock;

/*
 * Worker thread WITHOUT mutex:
 * Reads shared counter, yields CPU to simulate race condition,
 * then writes incremented value back.
 */
static void worker_no_mutex(void *arg)
{
    int id = *(int *)arg;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        int temp = counter_no_mutex;
        printf("[No Mutex]   Thread %d read counter = %d\n", id, temp);

        /* Explicitly yield to induce interleaved context switch */
        mt_yield();

        counter_no_mutex = temp + 1;
        printf("[No Mutex]   Thread %d updated counter to %d\n", id, counter_no_mutex);

        mt_yield();
    }

    mt_exit();
}

/*
 * Worker thread WITH mutex:
 * Acquires mutex before critical section, ensuring mutual exclusion.
 */
static void worker_with_mutex(void *arg)
{
    int id = *(int *)arg;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        mt_mutex_lock(&lock);

        int temp = counter_with_mutex;
        printf("[With Mutex] Thread %d locked mutex, read counter = %d\n", id, temp);

        /* Context switch inside critical section */
        mt_yield();

        counter_with_mutex = temp + 1;
        printf("[With Mutex] Thread %d updated counter to %d, unlocking\n", id, counter_with_mutex);

        mt_mutex_unlock(&lock);

        mt_yield();
    }

    mt_exit();
}

int main(void)
{
    int id1 = 1, id2 = 2;
    int id3 = 3, id4 = 4;
    int expected = NUM_ITERATIONS * 2;

    mt_init();

    printf("==================================================\n");
    printf(" DEMONSTRATION 1: RACE CONDITION (WITHOUT MUTEX)\n");
    printf("==================================================\n");

    int t1 = mt_create(worker_no_mutex, &id1);
    int t2 = mt_create(worker_no_mutex, &id2);

    mt_join(t1);
    mt_join(t2);

    printf("\n--- Result Without Mutex ---\n");
    printf("Expected Counter: %d\n", expected);
    printf("Actual Counter:   %d\n", counter_no_mutex);
    if (counter_no_mutex != expected) {
        printf(">> Race condition occurred! Value was lost due to interleaving.\n\n");
    } else {
        printf(">> Value matches expected.\n\n");
    }

    printf("==================================================\n");
    printf(" DEMONSTRATION 2: SYNCHRONIZATION (WITH MUTEX)\n");
    printf("==================================================\n");

    mt_mutex_init(&lock);

    int t3 = mt_create(worker_with_mutex, &id3);
    int t4 = mt_create(worker_with_mutex, &id4);

    mt_join(t3);
    mt_join(t4);

    printf("\n--- Result With Mutex ---\n");
    printf("Expected Counter: %d\n", expected);
    printf("Actual Counter:   %d\n", counter_with_mutex);
    if (counter_with_mutex == expected) {
        printf(">> SUCCESS: Mutual exclusion preserved! Counter is strictly correct.\n");
    } else {
        printf(">> FAILURE: Mutex did not protect critical section.\n");
    }
    printf("==================================================\n");

    return 0;
}
