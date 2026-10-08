#include "mthread.h"

#define MAX_QUEUE_SIZE 16

static int ready_queue[MAX_QUEUE_SIZE];
static int front = 0;
static int rear = 0;
static int count = 0;

/*
 * Initialize the ready queue.
 */
void mt_queue_init(void)
{
    int i;
    front = 0;
    rear = 0;
    count = 0;
    for (i = 0; i < MAX_QUEUE_SIZE; i++) {
        ready_queue[i] = -1;
    }
}

/*
 * Add a thread to the ready queue.
 * Rejects invalid IDs, full queue, and duplicate entries.
 */
int mt_queue_push(int thread_id)
{
    int i;

    if (thread_id < 0) {
        return -1;
    }

    if (count >= MAX_QUEUE_SIZE) {
        return -1;
    }

    /* Prevent duplicate insertion in ready queue */
    for (i = 0; i < count; i++) {
        int idx = (front + i) % MAX_QUEUE_SIZE;
        if (ready_queue[idx] == thread_id) {
            return -1; /* Already in queue */
        }
    }

    ready_queue[rear] = thread_id;
    rear = (rear + 1) % MAX_QUEUE_SIZE;
    count++;

    return 0;
}

/*
 * Remove and return the next thread from the ready queue.
 * Returns -1 if the queue is empty.
 */
int mt_queue_pop(void)
{
    int thread_id;

    if (count == 0) {
        return -1;
    }

    thread_id = ready_queue[front];
    ready_queue[front] = -1;
    front = (front + 1) % MAX_QUEUE_SIZE;
    count--;

    return thread_id;
}

/*
 * Check whether the ready queue is empty.
 */
int mt_queue_is_empty(void)
{
    return count == 0;
}

/*
 * Return current number of threads in the ready queue.
 */
int mt_queue_size(void)
{
    return count;
}

/*
 * Check if a specific thread is in the ready queue.
 */
int mt_queue_contains(int thread_id)
{
    int i;

    if (count == 0 || thread_id < 0) {
        return 0;
    }

    for (i = 0; i < count; i++) {
        int idx = (front + i) % MAX_QUEUE_SIZE;
        if (ready_queue[idx] == thread_id) {
            return 1;
        }
    }

    return 0;
}

/*
 * Remove a specific thread from anywhere inside the ready queue.
 * Returns 0 if found and removed, -1 otherwise.
 */
int mt_queue_remove(int thread_id)
{
    int i, found_pos = -1;

    if (count == 0 || thread_id < 0) {
        return -1;
    }

    for (i = 0; i < count; i++) {
        int idx = (front + i) % MAX_QUEUE_SIZE;
        if (ready_queue[idx] == thread_id) {
            found_pos = i;
            break;
        }
    }

    if (found_pos == -1) {
        return -1;
    }

    /* Shift all elements following found_pos one step forward */
    for (i = found_pos; i < count - 1; i++) {
        int curr_idx = (front + i) % MAX_QUEUE_SIZE;
        int next_idx = (front + i + 1) % MAX_QUEUE_SIZE;
        ready_queue[curr_idx] = ready_queue[next_idx];
    }

    rear = (rear - 1 + MAX_QUEUE_SIZE) % MAX_QUEUE_SIZE;
    ready_queue[rear] = -1;
    count--;

    return 0;
}