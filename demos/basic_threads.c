#include <stdio.h>
#include "mthread.h"

void thread_function(void *arg)
{
    int id = *(int *)arg;

    printf("Thread %d: Started\n", id);

    mt_yield();

    printf("Thread %d: Resumed\n", id);

    mt_yield();

    printf("Thread %d: Finished\n", id);
}

int main(void)
{
    int value1 = 1;
    int value2 = 2;

    mt_init();

    printf("Main thread: Creating threads\n");

    int t1 = mt_create(thread_function, &value1);
    int t2 = mt_create(thread_function, &value2);

    printf("Created thread %d\n", t1);
    printf("Created thread %d\n", t2);

    printf("Main thread: Starting execution\n");

    mt_yield();

    printf("Main thread: Waiting for threads\n");

    mt_join(t1);
    mt_join(t2);

    printf("Main thread: All threads finished\n");

    return 0;
}