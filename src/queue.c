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
    front = 0;
    rear = 0;
    count = 0;
}


/*
 * Add a thread to the ready queue.
 */
int mt_queue_push(int thread_id)
{
    if (count >= MAX_QUEUE_SIZE) {
        return -1;
    }

    ready_queue[rear] = thread_id;

    rear = (rear + 1) % MAX_QUEUE_SIZE;

    count++;

    return 0;
}


/*
 * Remove and return the next thread
 * from the ready queue.
 */
int mt_queue_pop(void)
{
    int thread_id;

    if (count == 0) {
        return -1;
    }

    thread_id = ready_queue[front];

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