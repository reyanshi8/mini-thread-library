#include <stdio.h>
#include <assert.h>
#include "mthread.h"

int main(void)
{
    printf("=== Running Test: Ready Queue ===\n");

    mt_queue_init();
    assert(mt_queue_is_empty() == 1);
    assert(mt_queue_size() == 0);
    assert(mt_queue_pop() == -1);

    /* Push elements */
    assert(mt_queue_push(1) == 0);
    assert(mt_queue_push(2) == 0);
    assert(mt_queue_push(3) == 0);
    assert(mt_queue_size() == 3);
    assert(mt_queue_is_empty() == 0);

    /* Test duplicate prevention */
    assert(mt_queue_push(2) == -1);
    assert(mt_queue_size() == 3);

    /* Test contains */
    assert(mt_queue_contains(1) == 1);
    assert(mt_queue_contains(2) == 1);
    assert(mt_queue_contains(3) == 1);
    assert(mt_queue_contains(4) == 0);

    /* Test remove middle element */
    assert(mt_queue_remove(2) == 0);
    assert(mt_queue_size() == 2);
    assert(mt_queue_contains(2) == 0);

    /* Test FIFO popping after removal */
    assert(mt_queue_pop() == 1);
    assert(mt_queue_pop() == 3);
    assert(mt_queue_pop() == -1);
    assert(mt_queue_is_empty() == 1);

    /* Test queue capacity limit (MAX_QUEUE_SIZE = 16) */
    mt_queue_init();
    for (int i = 0; i < 16; i++) {
        assert(mt_queue_push(i) == 0);
    }
    assert(mt_queue_size() == 16);
    /* 17th element must fail */
    assert(mt_queue_push(16) == -1);

    /* Pop all and verify */
    for (int i = 0; i < 16; i++) {
        assert(mt_queue_pop() == i);
    }
    assert(mt_queue_is_empty() == 1);

    printf(">>> Ready Queue Tests PASSED! <<<\n\n");
    return 0;
}
