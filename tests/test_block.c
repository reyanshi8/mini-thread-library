#include <stdio.h>
#include <assert.h>
#include "mthread.h"

static int worker_stage = 0;

void blocking_worker(void *arg)
{
    (void)arg;
    int self_id = mt_self();

    worker_stage = 1;

    /* Block self: should switch back immediately to main */
    int res = mt_block(self_id);
    assert(res == 0);

    /* After being unblocked by main thread, execution resumes here */
    worker_stage = 2;
}

int main(void)
{
    printf("=== Running Test: Thread Block and Unblock ===\n");

    mt_init();

    int worker_id = mt_create(blocking_worker, NULL);
    assert(worker_id > 0);
    assert(mt_get_state(worker_id) == MT_READY);

    /* Yield to worker so it starts and blocks itself */
    mt_yield();

    /* Worker ran stage 1, then blocked itself */
    assert(worker_stage == 1);
    assert(mt_get_state(worker_id) == MT_BLOCKED);
    assert(mt_queue_contains(worker_id) == 0);

    /* Multiple yields in main should NOT run worker because it is blocked */
    mt_yield();
    mt_yield();
    assert(worker_stage == 1);
    assert(mt_get_state(worker_id) == MT_BLOCKED);

    /* Main thread unblocks the worker */
    assert(mt_unblock(worker_id) == 0);
    assert(mt_get_state(worker_id) == MT_READY);
    assert(mt_queue_contains(worker_id) == 1);

    /* Now worker should run to stage 2 */
    assert(mt_join(worker_id) == 0);
    assert(worker_stage == 2);
    assert(mt_get_state(worker_id) == MT_TERMINATED);

    /* Unblocking an already terminated thread must fail */
    assert(mt_unblock(worker_id) == -1);

    /* Blocking a terminated thread must fail */
    assert(mt_block(worker_id) == -1);

    printf(">>> Thread Block/Unblock Tests PASSED! <<<\n\n");
    return 0;
}
