#include <stdio.h>
#include <assert.h>
#include "mthread.h"

void dummy_worker(void *arg)
{
    (void)arg;
    mt_yield();
}

int main(void)
{
    printf("=== Running Test: Validation and Robustness ===\n");

    mt_init();

    /* Validation of invalid IDs */
    assert(mt_is_valid(-1) == 0);
    assert(mt_is_valid(100) == 0);
    assert(mt_is_valid(999) == 0);

    /* State of invalid IDs */
    assert(mt_get_state(-1) == MT_TERMINATED);
    assert(mt_get_state(999) == MT_TERMINATED);

    /* Joining invalid IDs or self */
    assert(mt_join(-1) == -1);
    assert(mt_join(999) == -1);
    assert(mt_join(mt_self()) == -1);

    /* Blocking/unblocking invalid IDs */
    assert(mt_block(-1) == -1);
    assert(mt_block(999) == -1);
    assert(mt_unblock(-1) == -1);
    assert(mt_unblock(999) == -1);

    /* Null function pointer creation */
    assert(mt_create(NULL, NULL) == -1);

    /* Thread limit boundary test (MAX_THREADS = 16, thread 0 is main) */
    int created_ids[15];
    for (int i = 0; i < 15; i++) {
        created_ids[i] = mt_create(dummy_worker, NULL);
        assert(created_ids[i] == (i + 1));
        assert(mt_is_valid(created_ids[i]) == 1);
    }

    assert(mt_get_thread_count() == 16);

    /* 17th thread (index 16) must be rejected */
    int overflow_id = mt_create(dummy_worker, NULL);
    assert(overflow_id == -1);
    assert(mt_get_thread_count() == 16);

    /* Clean up all threads */
    for (int i = 0; i < 15; i++) {
        assert(mt_join(created_ids[i]) == 0);
        assert(mt_get_state(created_ids[i]) == MT_TERMINATED);
    }

    printf(">>> Validation and Robustness Tests PASSED! <<<\n\n");
    return 0;
}
