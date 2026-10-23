#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "mthread.h"

#define CREATION_COUNT 500
#define HANDOFF_COUNT 50000
#define COUNTER_OPS 100000

/* Global counters */
static long mthread_counter = 0;
static mt_mutex_t m_lock;

static long pthread_counter = 0;
static pthread_mutex_t p_lock;

/* Timer helper returning milliseconds */
static double get_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000.0) + (ts.tv_nsec / 1000000.0);
}

/* ========================================================
 * 1. Thread Creation + Completion (500 threads)
 * ======================================================== */
static void dummy_mthread(void *arg) { (void)arg; mt_exit(); }
static void *dummy_pthread(void *arg) { (void)arg; return NULL; }

static double bench_mthread_creation(void)
{
    double start = get_time_ms();
    for (int i = 0; i < CREATION_COUNT; i++) {
        int tid = mt_create(dummy_mthread, NULL);
        if (tid != -1) mt_join(tid);
    }
    return get_time_ms() - start;
}

static double bench_posix_creation(void)
{
    double start = get_time_ms();
    for (int i = 0; i < CREATION_COUNT; i++) {
        pthread_t pt;
        pthread_create(&pt, NULL, dummy_pthread, NULL);
        pthread_join(pt, NULL);
    }
    return get_time_ms() - start;
}

/* ========================================================
 * 2. Repeated Thread Handoffs (50,000 Cooperating Yields)
 * ======================================================== */
static void yield_mthread(void *arg)
{
    int iters = *(int *)arg;
    for (int i = 0; i < iters; i++) {
        mt_yield();
    }
    mt_exit();
}

static void *yield_pthread(void *arg)
{
    int iters = *(int *)arg;
    for (int i = 0; i < iters; i++) {
        sched_yield();
    }
    return NULL;
}

static double bench_mthread_handoffs(void)
{
    int iters = HANDOFF_COUNT;
    double start = get_time_ms();
    int t1 = mt_create(yield_mthread, &iters);
    int t2 = mt_create(yield_mthread, &iters);
    mt_yield();
    mt_join(t1);
    mt_join(t2);
    return get_time_ms() - start;
}

static double bench_posix_handoffs(void)
{
    int iters = HANDOFF_COUNT;
    double start = get_time_ms();
    pthread_t pt1, pt2;
    pthread_create(&pt1, NULL, yield_pthread, &iters);
    pthread_create(&pt2, NULL, yield_pthread, &iters);
    pthread_join(pt1, NULL);
    pthread_join(pt2, NULL);
    return get_time_ms() - start;
}

/* ========================================================
 * 3. Synchronized Shared Counter (100,000 Lock/Unlock Ops)
 * ======================================================== */
static double bench_mthread_counter(void)
{
    mt_mutex_init(&m_lock);
    mthread_counter = 0;
    double start = get_time_ms();
    for (int i = 0; i < COUNTER_OPS; i++) {
        mt_mutex_lock(&m_lock);
        mthread_counter++;
        mt_mutex_unlock(&m_lock);
    }
    return get_time_ms() - start;
}

static double bench_posix_counter(void)
{
    pthread_mutex_init(&p_lock, NULL);
    pthread_counter = 0;
    double start = get_time_ms();
    for (int i = 0; i < COUNTER_OPS; i++) {
        pthread_mutex_lock(&p_lock);
        pthread_counter++;
        pthread_mutex_unlock(&p_lock);
    }
    return get_time_ms() - start;
}

int main(void)
{
    mt_init();

    printf("Running exact section 10 benchmarks (repeating trials)...\n\n");

    /* Warm-up */
    bench_mthread_creation();
    bench_posix_creation();

    /* Benchmark 1: Thread creation + completion */
    double mt_create_ms = bench_mthread_creation();
    double pt_create_ms = bench_posix_creation();

    /* Benchmark 2: Repeated thread handoffs */
    double mt_handoff_ms = bench_mthread_handoffs();
    double pt_handoff_ms = bench_posix_handoffs();

    /* Benchmark 3: Synchronized shared counter */
    double mt_sync_ms = bench_mthread_counter();
    double pt_sync_ms = bench_posix_counter();

    double mt_total_ms = mt_create_ms + mt_handoff_ms + mt_sync_ms;
    double pt_total_ms = pt_create_ms + pt_handoff_ms + pt_sync_ms;

    printf("========================================================================\n");
    printf("%-32s | %-20s | %-20s\n", "Benchmark", "User-Level Library", "POSIX Threads");
    printf("========================================================================\n");
    printf("%-32s | %17.2f ms | %17.2f ms\n", "Thread creation + completion", mt_create_ms, pt_create_ms);
    printf("%-32s | %17.2f ms | %17.2f ms\n", "Repeated thread handoffs", mt_handoff_ms, pt_handoff_ms);
    printf("%-32s | %17.2f ms | %17.2f ms\n", "Synchronized shared counter", mt_sync_ms, pt_sync_ms);
    printf("------------------------------------------------------------------------\n");
    printf("%-32s | %17.2f ms | %17.2f ms\n", "Total execution time", mt_total_ms, pt_total_ms);
    printf("========================================================================\n");

    return 0;
}
