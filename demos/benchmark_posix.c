#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "mthread.h"

#define NUM_OPS 100000

/* Global counters */
static long mthread_counter = 0;
static mt_mutex_t m_lock;

static long pthread_counter = 0;
static pthread_mutex_t p_lock;

/* Timer helper (microseconds) */
static double get_time_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000.0) + (ts.tv_nsec / 1000.0);
}

/* ========================================================
 * 1. Thread Creation & Join Benchmark
 * ======================================================== */
static void dummy_mthread(void *arg)
{
    (void)arg;
    mt_exit();
}

static void *dummy_pthread(void *arg)
{
    (void)arg;
    return NULL;
}

static void benchmark_creation(int count)
{
    printf("------------------------------------------------------------\n");
    printf("1. Thread Creation & Termination Benchmark (%d iterations)\n", count);
    printf("------------------------------------------------------------\n");

    /* Mini Thread Library */
    double start_mt = get_time_us();
    for (int i = 0; i < count; i++) {
        int tid = mt_create(dummy_mthread, NULL);
        if (tid != -1) {
            mt_join(tid);
        }
    }
    double elapsed_mt = get_time_us() - start_mt;
    double avg_mt = elapsed_mt / count;

    /* POSIX Threads */
    double start_pt = get_time_us();
    for (int i = 0; i < count; i++) {
        pthread_t pt;
        pthread_create(&pt, NULL, dummy_pthread, NULL);
        pthread_join(pt, NULL);
    }
    double elapsed_pt = get_time_us() - start_pt;
    double avg_pt = elapsed_pt / count;

    printf("User-Level Mini-Thread: Total = %10.2f us | Avg per thread = %6.2f us\n", elapsed_mt, avg_mt);
    printf("POSIX Kernel Thread   : Total = %10.2f us | Avg per thread = %6.2f us\n", elapsed_pt, avg_pt);
    if (avg_pt > avg_mt) {
        printf(">> Speedup: Mini-Thread is %.2fx FASTER than POSIX threads!\n", avg_pt / avg_mt);
    } else {
        printf(">> Ratio: POSIX is %.2fx relative to Mini-Thread\n", avg_mt / avg_pt);
    }
}

/* ========================================================
 * 2. Mutex Lock / Unlock Overhead Benchmark
 * ======================================================== */
static void benchmark_mutex(int ops)
{
    printf("\n------------------------------------------------------------\n");
    printf("2. Mutex Lock/Unlock Overhead Benchmark (%d operations)\n", ops);
    printf("------------------------------------------------------------\n");

    mt_mutex_init(&m_lock);
    pthread_mutex_init(&p_lock, NULL);

    /* Mini Thread Mutex */
    double start_mt = get_time_us();
    for (int i = 0; i < ops; i++) {
        mt_mutex_lock(&m_lock);
        mthread_counter++;
        mt_mutex_unlock(&m_lock);
    }
    double elapsed_mt = get_time_us() - start_mt;
    double avg_mt = (elapsed_mt * 1000.0) / ops; /* nanoseconds */

    /* POSIX Mutex */
    double start_pt = get_time_us();
    for (int i = 0; i < ops; i++) {
        pthread_mutex_lock(&p_lock);
        pthread_counter++;
        pthread_mutex_unlock(&p_lock);
    }
    double elapsed_pt = get_time_us() - start_pt;
    double avg_pt = (elapsed_pt * 1000.0) / ops; /* nanoseconds */

    printf("Mini-Thread Mutex: Total = %10.2f us | Avg per op = %6.2f ns\n", elapsed_mt, avg_mt);
    printf("POSIX Mutex      : Total = %10.2f us | Avg per op = %6.2f ns\n", elapsed_pt, avg_pt);
}

/* ========================================================
 * 3. Context Switch Overhead (Yield Benchmark)
 * ======================================================== */
static int yield_target = 50000;
static void yield_worker(void *arg)
{
    (void)arg;
    for (int i = 0; i < yield_target; i++) {
        mt_yield();
    }
    mt_exit();
}

static void benchmark_context_switch(void)
{
    printf("\n------------------------------------------------------------\n");
    printf("3. Context Switch Benchmark (50,000 Cooperating Yields)\n");
    printf("------------------------------------------------------------\n");

    double start_mt = get_time_us();
    int t1 = mt_create(yield_worker, NULL);
    int t2 = mt_create(yield_worker, NULL);
    mt_yield();
    mt_join(t1);
    mt_join(t2);
    double elapsed_mt = get_time_us() - start_mt;
    double per_switch_ns = (elapsed_mt * 1000.0) / (yield_target * 2);

    printf("Mini-Thread swapcontext: Total = %10.2f us | Per Context Switch = %6.2f ns\n", elapsed_mt, per_switch_ns);
    printf(">> User-space registers swapped without entering kernel mode!\n");
}

int main(void)
{
    mt_init();

    printf("============================================================\n");
    printf("   BENCHMARK: Mini User-Level Threads vs POSIX (pthreads)   \n");
    printf("============================================================\n\n");

    benchmark_creation(500);
    benchmark_mutex(NUM_OPS);
    benchmark_context_switch();

    printf("\n============================================================\n");
    printf("                 BENCHMARK COMPLETED                        \n");
    printf("============================================================\n");

    return 0;
}
