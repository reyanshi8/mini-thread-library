#include <stdio.h>
#include <unistd.h>
#include "mthread.h"
#include "sched.h"
#include "timer.h"

void cpu_bound_thread(void *arg)
{
    int id = *(int *)arg;
    long count = 0;

    printf("[Thread %d] Started (CPU bound, no voluntary yields)\n", id);

    for (int i = 0; i < 5; i++) {
        /* Busy wait loop to consume quantum */
        for (volatile long j = 0; j < 50000000L; j++) {
            count++;
        }
        printf("[Thread %d] Quantum step %d completed (count=%ld)\n", id, i + 1, count);
    }

    printf("[Thread %d] Finished\n", id);
}

int main(void)
{
    int id1 = 1, id2 = 2, id3 = 3;

    mt_init();

    printf("=== Mini Thread Library Preemption Demo ===\n");
    printf("Starting Round Robin scheduler with 10ms time quantum...\n");
    sched_init(10000); /* 10,000 microseconds = 10 ms quantum */

    int t1 = mt_create(cpu_bound_thread, &id1);
    int t2 = mt_create(cpu_bound_thread, &id2);
    int t3 = mt_create(cpu_bound_thread, &id3);

    printf("Created threads %d, %d, %d\n", t1, t2, t3);
    printf("Main thread yielding control...\n");

    mt_yield();

    mt_join(t1);
    mt_join(t2);
    mt_join(t3);

    printf("Main thread: All preemptive threads finished successfully!\n");

    return 0;
}
