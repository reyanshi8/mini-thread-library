# Mini User-Level Thread Library (`mthread`)

A lightweight user-level threading library implemented in C utilizing POSIX context switching (`ucontext`). Developed as part of the Operating Systems course project.

---

## 👥 Project Team Division

| Role | Member | Responsibilities | Module Files | Status |
|---|---|---|---|---|
| **Person 1** | **Thread Management** | TCB, Stacks, Context Switching, Ready Queue, Thread States, Lifecycle (`create`, `yield`, `exit`, `join`, `block`, `unblock`), Validation & Tests | `include/mthread.h`<br>`src/thread.c`<br>`src/queue.c`<br>`tests/` | **COMPLETE** ✅ |
| **Person 2** | **CPU Scheduling & Preemption** | Round Robin Scheduling, Timer-based Preemption (`SIGALRM`, `setitimer`), Quantum handling | `src/sched.c`<br>`src/timer.c` | In Progress / Next |
| **Person 3** | **Synchronization & Demos** | Mutex (`mt_mutex`), Semaphore (`mt_sem`), Race Condition Demo, Producer-Consumer Demo | `src/sync.c`<br>`demos/race_condition.c`<br>`demos/producer_consumer.c` | In Progress / Next |

---

## 📌 Person 1 Architecture & Design

### 1. Thread Control Block (TCB)
Each thread is tracked via a Thread Control Block:
```c
typedef struct {
    int id;                   /* Unique thread identifier (0 to MAX_THREADS-1) */
    mt_thread_state_t state;  /* Current execution state */
    ucontext_t context;       /* Saved CPU execution context */
    void *stack;              /* Allocated execution stack */
    void (*function)(void *); /* Thread entry point */
    void *arg;                /* Argument passed to entry point */
} TCB;
```

### 2. Thread State Lifecycle
Threads transition across 4 distinct states:
```
                [ mt_create() ]
                       │
                       ▼
                 ┌───────────┐
                 │ MT_READY  │ ◄──────────────────────┐
                 └─────┬─────┘                        │
                       │ [scheduled / mt_yield]       │ [mt_unblock()]
                       ▼                              │
                 ┌───────────┐  [mt_block()]    ┌─────┴──────┐
                 │MT_RUNNING ├─────────────────►│ MT_BLOCKED │
                 └─────┬─────┘                  └────────────┘
                       │ [mt_exit() / return]
                       ▼
               ┌───────────────┐
               │ MT_TERMINATED │
               └───────────────┘
```

- **`MT_READY`**: Enqueued in the FIFO Ready Queue waiting for CPU time.
- **`MT_RUNNING`**: Actively executing on the CPU (only 1 thread at a time).
- **`MT_BLOCKED`**: Suspended waiting on synchronization resources (mutex lock, semaphore wait); not present in Ready Queue.
- **`MT_TERMINATED`**: Execution finished. Memory/stack is safely reclaimed by subsequent threads to avoid freeing an active stack.

### 3. Circular Ready Queue
Implemented in `src/queue.c` using a circular FIFO buffer:
- **Duplicate Prevention**: Prevents the same thread ID from being queued multiple times.
- **Bounded Capacity**: Supports up to `MAX_QUEUE_SIZE` elements with overflow protection.
- **Removal Support**: Supports arbitrary thread removal (`mt_queue_remove`) when a ready thread is blocked.

---

## 🛠 Public API Reference (`include/mthread.h`)

### Core Thread Management
- `void mt_init(void)`: Initializes the thread system and registers the main process thread (ID 0).
- `int mt_create(void (*func)(void *), void *arg)`: Allocates stack, initializes context, and schedules a new thread. Returns thread ID or -1 on error.
- `void mt_yield(void)`: Voluntarily yields the CPU to the next ready thread in the queue.
- `void mt_exit(void)`: Terminates the current thread and safely transfers execution to the next ready thread.
- `int mt_join(int thread_id)`: Blocks the caller until `thread_id` terminates.
- `int mt_self(void)` / `int mt_get_current(void)`: Returns the ID of the calling thread.
- `mt_thread_state_t mt_get_state(int thread_id)`: Returns current state of the thread.
- `const char *mt_state_to_str(mt_thread_state_t state)`: Helper returning string representation (`"READY"`, `"RUNNING"`, etc.).
- `int mt_is_valid(int thread_id)`: Checks if a thread ID exists and is within valid bounds.
- `int mt_get_thread_count(void)`: Returns total number of threads registered.

### Synchronization Hooks (For Person 3)
- `int mt_block(int thread_id)`: Changes thread state to `MT_BLOCKED`. If `thread_id` is the current thread, automatically yields the CPU to the next ready thread.
- `int mt_unblock(int thread_id)`: Moves thread state from `MT_BLOCKED` to `MT_READY` and re-enqueues it into the ready queue.

### Scheduler Hooks (For Person 2)
- `void mt_schedule(void)`: Hook to trigger scheduling switch (e.g. on `SIGALRM` timer interrupt).
- Ready Queue APIs: `mt_queue_push`, `mt_queue_pop`, `mt_queue_is_empty`, `mt_queue_size`, `mt_queue_contains`, `mt_queue_remove`.

---

## 🚀 Building and Running

### Prerequisites
- GCC / Clang
- Make
- POSIX-compliant environment (macOS, Linux, WSL)

### Compilation
```bash
# Build all demos
make all

# Run basic threads demonstration
./basic_threads

# Run comprehensive Person 1 module demonstration
./demo_person1
```

### Running Test Suite
Person 1 includes an automated test suite verifying queue logic, lifecycle, synchronization hooks, and edge cases:
```bash
make test
```

Test coverage:
1. `test_queue`: Circular queue FIFO order, duplicate prevention, overflow handling, search, and arbitrary removal.
2. `test_lifecycle`: Multi-threaded creation, cooperative execution, interleaving, and join synchronization.
3. `test_block`: Block/unblock state transitions and verification that blocked threads do not consume CPU time until unblocked.
4. `test_validation`: Out-of-bounds IDs, joining self, joining non-existent threads, null pointers, and `MAX_THREADS` capacity enforcement.

---

## 🤝 Integration Guidelines for Teammates

### For Person 2 (Scheduling & Preemption):
1. Use `mt_queue_pop()` and `mt_queue_push(id)` for Round Robin queue rotations.
2. Inside your `SIGALRM` handler in `src/timer.c`, call `mt_yield()` or `mt_schedule()` to trigger preemption.
3. Use `mt_get_current()` to inspect the running thread ID.

### For Person 3 (Synchronization):
1. In `mt_mutex_lock(mt_mutex_t *mutex)`: If mutex is locked, add `mt_self()` to your mutex wait list and call `mt_block(mt_self())`.
2. In `mt_mutex_unlock(mt_mutex_t *mutex)`: Pop the waiting thread ID from your wait list and call `mt_unblock(waiting_id)`.
3. In `mt_sem_wait(mt_sem_t *sem)` and `mt_sem_post(mt_sem_t *sem)`: Follow the same pattern with `mt_block(mt_self())` and `mt_unblock(waiting_id)`.
4. No need to modify `thread.c` or access the internal TCB! The public API handles state transitions and stack safety cleanly.
