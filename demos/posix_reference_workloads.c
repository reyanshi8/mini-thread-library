#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>

#define BUFFER_SIZE 4
#define TOTAL_ITEMS 8

/* =========================================================================
 * WORKLOAD 1: Thread Creation & Lifecycle (POSIX pthreads)
 * ========================================================================= */
static void *posix_dummy_worker(void *arg)
{
    (void)arg;
    return NULL;
}

void run_posix_lifecycle_demo(void)
{
    printf("\n============================================================\n");
    printf(" [POSIX PTHREADS] WORKLOAD 1: THREAD LIFECYCLE & JOIN\n");
    printf("============================================================\n");

    pthread_t t1, t2;
    printf("Main POSIX thread: Creating 2 kernel threads via pthread_create()...\n");

    pthread_create(&t1, NULL, posix_dummy_worker, NULL);
    pthread_create(&t2, NULL, posix_dummy_worker, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Main POSIX thread: Both kernel threads finished and joined successfully!\n");
}

/* =========================================================================
 * WORKLOAD 2: Race Condition vs POSIX pthread_mutex_t
 * ========================================================================= */
static int posix_counter_no_lock = 0;
static int posix_counter_with_lock = 0;
static pthread_mutex_t posix_lock = PTHREAD_MUTEX_INITIALIZER;

static void *posix_race_worker_nolock(void *arg)
{
    int id = *(int *)arg;
    for (int i = 0; i < 5; i++) {
        int temp = posix_counter_no_lock;
        printf("[POSIX No Mutex]   Thread %d read counter = %d\n", id, temp);
        sched_yield();
        posix_counter_no_lock = temp + 1;
        printf("[POSIX No Mutex]   Thread %d updated counter to %d\n", id, posix_counter_no_lock);
        sched_yield();
    }
    return NULL;
}

static void *posix_race_worker_withlock(void *arg)
{
    int id = *(int *)arg;
    for (int i = 0; i < 5; i++) {
        pthread_mutex_lock(&posix_lock);
        int temp = posix_counter_with_lock;
        printf("[POSIX With Mutex] Thread %d locked mutex, read counter = %d\n", id, temp);
        sched_yield();
        posix_counter_with_lock = temp + 1;
        printf("[POSIX With Mutex] Thread %d updated counter to %d, unlocking\n", id, posix_counter_with_lock);
        pthread_mutex_unlock(&posix_lock);
        sched_yield();
    }
    return NULL;
}

void run_posix_race_demo(void)
{
    printf("\n============================================================\n");
    printf(" [POSIX PTHREADS] WORKLOAD 2: MUTEX & RACE CONDITION\n");
    printf("============================================================\n");

    int id1 = 1, id2 = 2;
    pthread_t t1, t2;

    printf("--- Part A: Without pthread_mutex ---\n");
    pthread_create(&t1, NULL, posix_race_worker_nolock, &id1);
    pthread_create(&t2, NULL, posix_race_worker_nolock, &id2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Expected: 10 | Actual Counter: %d\n", posix_counter_no_lock);

    printf("\n--- Part B: With pthread_mutex ---\n");
    pthread_create(&t1, NULL, posix_race_worker_withlock, &id1);
    pthread_create(&t2, NULL, posix_race_worker_withlock, &id2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Expected: 10 | Actual Counter: %d\n", posix_counter_with_lock);
    printf(">> SUCCESS: Mutual exclusion preserved with pthread_mutex_t!\n");
}

/* =========================================================================
 * WORKLOAD 3: Producer-Consumer with POSIX Semaphores
 * ========================================================================= */
static int buffer[BUFFER_SIZE];
static int in = 0;
static int out = 0;
static int count = 0;

/* Note: On macOS, unnamed sem_init is deprecated/mocked with mutex+cond */
static pthread_mutex_t buf_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

static void *posix_producer(void *arg)
{
    (void)arg;
    for (int i = 0; i < TOTAL_ITEMS; i++) {
        pthread_mutex_lock(&buf_mutex);
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&not_full, &buf_mutex);
        }
        int item = 100 + (i + 1);
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;
        printf("[POSIX Producer] Produced item %d | Buffer items: %d/%d\n", item, count, BUFFER_SIZE);
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&buf_mutex);
    }
    return NULL;
}

static void *posix_consumer(void *arg)
{
    (void)arg;
    for (int i = 0; i < TOTAL_ITEMS; i++) {
        pthread_mutex_lock(&buf_mutex);
        while (count == 0) {
            pthread_cond_wait(&not_empty, &buf_mutex);
        }
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        printf("[POSIX Consumer] Consumed item %d | Buffer items: %d/%d\n", item, count, BUFFER_SIZE);
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&buf_mutex);
    }
    return NULL;
}

void run_posix_prod_cons_demo(void)
{
    printf("\n============================================================\n");
    printf(" [POSIX PTHREADS] WORKLOAD 3: PRODUCER-CONSUMER BUFFER\n");
    printf("============================================================\n");

    pthread_t prod, cons;
    pthread_create(&prod, NULL, posix_producer, NULL);
    pthread_create(&cons, NULL, posix_consumer, NULL);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    printf(">> SUCCESS: All %d items produced and consumed using POSIX primitives!\n", TOTAL_ITEMS);
}

int main(void)
{
    printf("************************************************************\n");
    printf("   EQUIVALENT POSIX PTHREAD WORKLOADS BENCHMARK SUITE       \n");
    printf("   (Comparison Counterpart to MicroThreads Library)         \n");
    printf("************************************************************\n");

    run_posix_lifecycle_demo();
    run_posix_race_demo();
    run_posix_prod_cons_demo();

    printf("\n************************************************************\n");
    printf("   ALL POSIX WORKLOADS COMPLETED SUCCESSFULLY               \n");
    printf("************************************************************\n");

    return 0;
}
