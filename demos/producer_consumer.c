#include <stdio.h>
#include "mthread.h"

#define BUFFER_SIZE 4
#define TOTAL_ITEMS 8

/* Bounded Buffer state */
static int buffer[BUFFER_SIZE];
static int in = 0;
static int out = 0;
static int count = 0;

/* Synchronization primitives */
static mt_mutex_t buffer_mutex;
static mt_sem_t empty_slots;
static mt_sem_t full_slots;

/*
 * Producer Thread:
 * Generates items, waits for an empty slot in the buffer,
 * locks the mutex, inserts the item, unlocks, and signals full slot.
 */
static void producer_thread(void *arg)
{
    int id = *(int *)arg;

    for (int i = 1; i <= TOTAL_ITEMS; i++) {
        int item = (id * 100) + i;

        /* Wait for space in buffer */
        mt_sem_wait(&empty_slots);

        /* Enter critical section */
        mt_mutex_lock(&buffer_mutex);

        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;

        printf("[Producer %d] Produced item %3d | Buffer items: %d/%d\n",
               id, item, count, BUFFER_SIZE);

        mt_mutex_unlock(&buffer_mutex);

        /* Signal available item */
        mt_sem_post(&full_slots);

        mt_yield();
    }

    mt_exit();
}

/*
 * Consumer Thread:
 * Waits for an item in the buffer, locks the mutex,
 * extracts the item, unlocks, and signals empty slot.
 */
static void consumer_thread(void *arg)
{
    int id = *(int *)arg;

    for (int i = 1; i <= TOTAL_ITEMS; i++) {
        /* Wait for item in buffer */
        mt_sem_wait(&full_slots);

        /* Enter critical section */
        mt_mutex_lock(&buffer_mutex);

        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;

        printf("[Consumer %d] Consumed item %3d | Buffer items: %d/%d\n",
               id, item, count, BUFFER_SIZE);

        mt_mutex_unlock(&buffer_mutex);

        /* Signal available space */
        mt_sem_post(&empty_slots);

        mt_yield();
    }

    mt_exit();
}

int main(void)
{
    int prod_id = 1;
    int cons_id = 1;

    mt_init();

    printf("====================================================\n");
    printf(" PRODUCER-CONSUMER DEMONSTRATION (BOUNDED BUFFER)\n");
    printf(" Buffer Capacity: %d | Total Items: %d\n", BUFFER_SIZE, TOTAL_ITEMS);
    printf("====================================================\n");

    /* Initialize synchronization primitives */
    mt_mutex_init(&buffer_mutex);
    mt_sem_init(&empty_slots, BUFFER_SIZE); /* Initially all slots are empty */
    mt_sem_init(&full_slots, 0);            /* Initially no items in buffer */

    int t_prod = mt_create(producer_thread, &prod_id);
    int t_cons = mt_create(consumer_thread, &cons_id);

    printf("Created Producer thread %d and Consumer thread %d\n\n", t_prod, t_cons);

    mt_join(t_prod);
    mt_join(t_cons);

    printf("\n====================================================\n");
    printf(">> SUCCESS: All %d items successfully produced and consumed!\n", TOTAL_ITEMS);
    printf("Final buffer count: %d (Expected: 0)\n", count);
    printf("====================================================\n");

    return 0;
}
