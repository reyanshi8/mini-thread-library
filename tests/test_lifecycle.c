#include <stdio.h>
#include <assert.h>
#include "mthread.h"

static int counter = 0;
static int thread_order[6];
static int order_idx = 0;

void worker(void *arg)
{
    int id = *(int *)arg;

    assert(mt_self() == id);
    assert(mt_get_state(id) == MT_RUNNING);

    thread_order[order_idx++] = id;
    counter++;

    /* Voluntarily yield CPU */
    mt_yield();

    thread_order[order_idx++] = id;
    counter++;

    /* Thread finishes and terminates automatically */
}

int main(void)
{
    printf("=== Running Test: Thread Lifecycle (Create, Yield, Join, Exit) ===\n");

    mt_init();

    assert(mt_self() == 0);
    assert(mt_get_state(0) == MT_RUNNING);

    int id1 = 1;
    int id2 = 2;
    int id3 = 3;

    int t1 = mt_create(worker, &id1);
    int t2 = mt_create(worker, &id2);
    int t3 = mt_create(worker, &id3);

    assert(t1 == 1);
    assert(t2 == 2);
    assert(t3 == 3);
    assert(mt_get_thread_count() == 4);

    assert(mt_get_state(t1) == MT_READY);
    assert(mt_get_state(t2) == MT_READY);
    assert(mt_get_state(t3) == MT_READY);

    /* Let threads run cooperatively */
    mt_yield();

    /* Join all threads */
    assert(mt_join(t1) == 0);
    assert(mt_join(t2) == 0);
    assert(mt_join(t3) == 0);

    /* All threads must be terminated */
    assert(mt_get_state(t1) == MT_TERMINATED);
    assert(mt_get_state(t2) == MT_TERMINATED);
    assert(mt_get_state(t3) == MT_TERMINATED);

    /* Verify execution counter: 3 threads * 2 steps = 6 */
    assert(counter == 6);
    assert(order_idx == 6);

    printf("Thread execution sequence: ");
    for (int i = 0; i < 6; i++) {
        printf("T%d ", thread_order[i]);
    }
    printf("\n");

    printf(">>> Thread Lifecycle Tests PASSED! <<<\n\n");
    return 0;
}
