# Presentation Slides Outline (PPT)
## Project: Mini User-Level Thread Library vs. POSIX Threads
**Team:** Person 1 (Thread Mgmt), Person 2 (Scheduling & Preemption), Person 3 (Synchronization)

---

### Slide 1: Title Slide
- **Project Title:** Design & Evaluation of a Preemptive User-Level Thread Library in C
- **Course:** Operating Systems
- **Team Members:** Person 1, Person 2, Person 3
- **Repository:** https://github.com/reyanshi8/mini-thread-library

---

### Slide 2: Problem Statement & Motivation
- **Context:** Understanding multi-threading mechanisms from first principles.
- **Goal:** Build an M:1 User-Level Threading library from scratch without relying on `pthread_create()`.
- **Key Challenges:**
  - Stack allocation and register switching in user space (`ucontext_t`).
  - Achieving preemption without kernel scheduler support (via `SIGALRM` & `setitimer`).
  - Mutual exclusion and condition synchronization.

---

### Slide 3: Team Division & Architecture
- **Person 1 (Thread Management):**
  - TCB (Thread Control Block) state transitions: `READY`, `RUNNING`, `BLOCKED`, `TERMINATED`.
  - Circular FIFO Ready Queue (`src/queue.c`).
  - Core API: `mt_create()`, `mt_yield()`, `mt_exit()`, `mt_join()`.
- **Person 2 (Scheduling & Preemption):**
  - Round-Robin scheduler with configurable time quantum (`src/sched.c`).
  - Real-time interval timer preemption (`SIGALRM` / `setitimer()`) (`src/timer.c`).
- **Person 3 (Synchronization Primitives):**
  - Mutual exclusion lock (`mt_mutex_t`) (`src/sync.c`).
  - Counting semaphore (`mt_sem_t`).

---

### Slide 4: Core Execution Flow (How It Works)
- **Thread Creation:** `malloc(STACK_SIZE)` -> `getcontext()` -> `makecontext()`.
- **Preemption:**
  - Timer counts down time quantum (10 ms).
  - Timer expires -> OS sends `SIGALRM`.
  - Signal handler calls `sched_tick()`.
  - `sched_tick()` masks signal -> calls `mt_yield()` -> `swapcontext()`.
  - Next thread executes.

---

### Slide 5: Empirical Benchmark vs POSIX Threads (`pthreads`)
*(Include the generated graph `benchmark_results.png` here)*
- **Thread Creation & Termination Latency:**
  - User-Level Mini-Thread: **4.12 μs** per thread.
  - POSIX Kernel Thread: **32.49 μs** per thread.
  - **Result: Mini-Thread is ~7.9x faster** because it avoids kernel traps (`clone()` syscall) and kernel `task_struct` allocation.
- **Mutex Overhead:**
  - User-space mutex: **11.23 ns** vs POSIX mutex: **15.84 ns**.

---

### Slide 6: Live Demonstrations Overview
1. **`./preemption_demo`**: CPU-bound loops without voluntary yield preempted automatically.
2. **`./race_condition`**: Demonstrates data loss (5/10) without mutex vs guaranteed consistency (10/10) with mutex.
3. **`./producer_consumer`**: Bounded buffer synchronization with semaphores.
4. **`make test`**: 100% pass on all 4 unit test suites.

---

### Slide 7: Conclusion & Lessons Learned
- User-level threads offer significantly lower creation and context-switch latency.
- Trade-offs: User-level threads in an M:1 model cannot achieve multi-core hardware parallelism and require signal tricks for preemption.
- Successfully implemented, tested, and benchmarked against standard POSIX threads!
