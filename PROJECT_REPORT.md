# Project Report: Mini User-Level Thread Library vs. POSIX Threads

**Course:** Operating Systems  
**Project:** User-Level Thread Library Design, Preemption, Synchronization & POSIX Benchmarking  
**Repository:** [github.com/reyanshi8/mini-thread-library](https://github.com/reyanshi8/mini-thread-library)  

---

## 1. Executive Summary & Objective

In this project, we designed and implemented an M:1 User-Level Threading Library in C that provides:
1. **Thread Lifecycle Management:** Thread creation, cooperative yielding, graceful termination, and joining.
2. **Ready Queue & State Machine:** Robust circular queue maintaining thread control blocks (`TCB`) transitioning between `READY`, `RUNNING`, `BLOCKED`, and `TERMINATED`.
3. **Round-Robin Scheduling & Preemption:** Configurable time quantum (default 10 ms) with hardware interval timer interrupts (`setitimer` generating `SIGALRM`).
4. **Synchronization Primitives:** Mutual exclusion locks (`mt_mutex_t`) and Counting Semaphores (`mt_sem_t`) to eliminate race conditions and implement bounded buffers.
5. **POSIX Comparison Benchmark:** Empirical measurement of user-space context switching & thread creation latency vs. kernel-level POSIX threads (`pthreads`).

---

## 2. Team Division & Module Integration (Rules 6, 7 & 8)

| Module | Member | Key Responsibilities & Owned Files |
| :--- | :--- | :--- |
| **Person 1** | Thread Management & Core API | `src/thread.c`, `src/queue.c`, `include/mthread.h`, unit tests (`tests/`) |
| **Person 2** | Scheduling & Preemption | `src/sched.c`, `src/timer.c`, `include/sched.h`, `include/timer.h`, `demos/preemption_demo.c` |
| **Person 3** | Synchronization & Concurrency Demos | `src/sync.c`, `demos/race_condition.c`, `demos/producer_consumer.c` |

All three branches were merged into `main` without file ownership conflicts.

---

## 3. Technical Architecture

### 3.1 Context Switching Mechanism
User-level execution context is saved and restored using System V POSIX context structures (`ucontext_t`):
- `getcontext()`: Snapshots current CPU registers.
- `makecontext()`: Configures a thread wrapper entry point and dedicated 64 KB stack.
- `swapcontext()`: Atomically saves caller registers and switches stack pointer (`SP`) and program counter (`PC`) to the target thread in user space.

### 3.2 Preemptive Scheduling (Timer Interrupts)
To prevent greedy CPU-bound threads from starving others, Person 2 incorporated:
- Interval timer: `setitimer(ITIMER_REAL, ...)` configured to fire every 10,000 μs (10 ms).
- Signal handler: Captures `SIGALRM` and invokes `sched_tick()`.
- Signal masking: `sigprocmask(SIG_BLOCK, &SIGALRM)` shields critical queue operations and context switches to prevent re-entrant corruptions.

---

## 4. Empirical Performance Benchmarks (User-Level vs POSIX pthreads)

We designed a comprehensive benchmark suite (`demos/benchmark_posix.c`) measuring 3 primary performance dimensions:

| Benchmark Dimension | Our User-Level Mini-Thread | POSIX Kernel Thread (`pthread`) | Performance Advantage |
| :--- | :---: | :---: | :---: |
| **Thread Creation & Join (500 iters)** | **4.12 μs** / thread | **32.49 μs** / thread | **Mini-Thread is ~7.9x Faster** |
| **Mutex Lock + Unlock (100,000 ops)** | **11.23 ns** / op | **15.84 ns** / op | **Mini-Thread is ~1.4x Faster** |
| **Context Switch Overhead** | **902.82 ns** (User space) | System Call Trap Overhead | **Zero Kernel Traps** |

### Why User-Level Threads Outperform POSIX `pthreads` in Creation:
1. **Zero System Calls:** `mt_create()` allocates user-space stack memory and sets registers without trapping to the kernel.
2. **No Kernel Object Creation:** POSIX `pthread_create()` invokes the Linux `clone()` system call, allocating a kernel `task_struct`, kernel stack, and registering with the OS scheduler.

---

## 5. Demonstration Suites & Validation

1. **Preemption Demo (`./preemption_demo`):**
   Three CPU-bound threads execute infinite loops with no voluntary `mt_yield()`. The timer automatically slices execution evenly among all threads.
2. **Race Condition & Mutex Demo (`./race_condition`):**
   Unsynchronized counter yields data loss (expected 10, actual 5). With `mt_mutex_t`, counter reaches exact expected 10/10.
3. **Producer-Consumer Demo (`./producer_consumer`):**
   Demonstrates bounded buffer coordination with 2 counting semaphores (`empty_slots`, `full_slots`) and 1 mutex lock.
4. **Automated Unit Test Suite (`make test`):**
   Runs 4 test suites (`test_queue`, `test_lifecycle`, `test_block`, `test_validation`) validating queue wrap-around, parameter edge cases, and thread states.
