#include <stdio.h>
#include "mthread.h"

void compute_task(void *arg)
{
    int id = *(int *)arg;

    printf("[Thread %d] Started! State: %s, Current: %d\n",
           id, mt_state_to_str(mt_get_state(id)), mt_self());

    for (int step = 1; step <= 3; step++) {
        printf("[Thread %d] Step %d/3 - Yielding CPU (Queue size: %d)...\n",
               id, step, mt_queue_size());
        mt_yield();
        printf("[Thread %d] Resumed at step %d!\n", id, step);
    }

    printf("[Thread %d] Finished task, exiting!\n", id);
}

void blocking_task(void *arg)
{
    int id = *(int *)arg;

    printf("[Worker %d] Started. Simulating waiting on external event...\n", id);
    printf("[Worker %d] Blocking self (State before block: %s)...\n",
           id, mt_state_to_str(mt_get_state(id)));

    mt_block(id);

    printf("[Worker %d] Unblocked! Resuming execution (State: %s)...\n",
           id, mt_state_to_str(mt_get_state(id)));
}

int main(void)
{
    printf("=====================================================\n");
    printf("  Mini-Thread Library: Person 1 Demonstration\n");
    printf("  Thread Management, States & Ready Queue\n");
    printf("=====================================================\n\n");

    mt_init();
    printf("[Main] Initialized. Main thread ID: %d, State: %s\n\n",
           mt_self(), mt_state_to_str(mt_get_state(mt_self())));

    /* Part 1: Cooperative Multithreading */
    printf("--- Part 1: Thread Creation & Cooperative Scheduling ---\n");
    int val1 = 1, val2 = 2;
    int t1 = mt_create(compute_task, &val1);
    int t2 = mt_create(compute_task, &val2);

    printf("[Main] Created Thread %d (State: %s)\n", t1, mt_state_to_str(mt_get_state(t1)));
    printf("[Main] Created Thread %d (State: %s)\n", t2, mt_state_to_str(mt_get_state(t2)));
    printf("[Main] Ready queue contains T1: %d, T2: %d. Queue size: %d\n\n",
           mt_queue_contains(t1), mt_queue_contains(t2), mt_queue_size());

    printf("[Main] Yielding CPU to worker threads...\n");
    mt_yield();

    printf("\n[Main] Waiting for Thread %d and Thread %d to terminate...\n", t1, t2);
    mt_join(t1);
    mt_join(t2);

    printf("[Main] Thread %d state: %s, Thread %d state: %s\n\n",
           t1, mt_state_to_str(mt_get_state(t1)),
           t2, mt_state_to_str(mt_get_state(t2)));

    /* Part 2: Thread Blocking and Unblocking */
    printf("--- Part 2: State Transitions (MT_BLOCKED & MT_READY) ---\n");
    int val3 = 3;
    int t3 = mt_create(blocking_task, &val3);
    printf("[Main] Created Worker Thread %d\n", t3);

    /* Yield so t3 runs and blocks itself */
    mt_yield();

    printf("[Main] Worker %d state is now: %s (Queue contains T3: %d)\n",
           t3, mt_state_to_str(mt_get_state(t3)), mt_queue_contains(t3));

    printf("[Main] Simulating resource availability... Unblocking Worker %d\n", t3);
    mt_unblock(t3);
    printf("[Main] Worker %d state after unblock: %s (Queue contains T3: %d)\n",
           t3, mt_state_to_str(mt_get_state(t3)), mt_queue_contains(t3));

    mt_join(t3);
    printf("[Main] Worker %d joined! State: %s\n\n",
           t3, mt_state_to_str(mt_get_state(t3)));

    printf("=====================================================\n");
    printf("  Person 1 Module Demonstration Completed Successfully!\n");
    printf("=====================================================\n");

    return 0;
}
