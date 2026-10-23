import os
import docx
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml import parse_xml
from docx.oxml.ns import nsdecls

def create_report():
    doc = docx.Document()

    # Set standard margins
    for section in doc.sections:
        section.top_margin = Inches(1)
        section.bottom_margin = Inches(1)
        section.left_margin = Inches(1)
        section.right_margin = Inches(1)

    # Palette
    NAVY = RGBColor(26, 82, 118)
    DARK_GRAY = RGBColor(44, 62, 80)
    BLACK = RGBColor(0, 0, 0)

    # Title
    p_title = doc.add_paragraph()
    p_title.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_title.paragraph_format.space_before = Pt(0)
    p_title.paragraph_format.space_after = Pt(4)
    run_title = p_title.add_run("PROJECT REPORT")
    run_title.font.name = "Arial"
    run_title.font.size = Pt(24)
    run_title.font.bold = True
    run_title.font.color.rgb = NAVY

    p_sub = doc.add_paragraph()
    p_sub.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_sub.paragraph_format.space_after = Pt(18)
    run_sub = p_sub.add_run("Design, Implementation & Empirical POSIX Benchmarking\nof an M:1 Preemptive User-Level Thread Library in C")
    run_sub.font.name = "Arial"
    run_sub.font.size = Pt(14)
    run_sub.font.bold = True
    run_sub.font.color.rgb = DARK_GRAY

    # Meta Table (Course, Team, Repo)
    meta_table = doc.add_table(rows=4, cols=2)
    meta_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    meta_data = [
        ("Course:", "Operating Systems (CSE)"),
        ("Project:", "Mini User-Level Thread Library (M:1 Model)"),
        ("Team Members:", "Person 1 (Thread Mgmt), Person 2 (Scheduling & Preemption), Person 3 (Synchronization)"),
        ("GitHub Repository:", "https://github.com/reyanshi8/mini-thread-library")
    ]
    for row_idx, (k, v) in enumerate(meta_data):
        cell_k = meta_table.cell(row_idx, 0)
        cell_v = meta_table.cell(row_idx, 1)
        cell_k.text = k
        cell_v.text = v
        cell_k.paragraphs[0].runs[0].font.bold = True
        cell_k.paragraphs[0].runs[0].font.name = "Arial"
        cell_v.paragraphs[0].runs[0].font.name = "Arial"
        cell_k.paragraphs[0].runs[0].font.size = Pt(10)
        cell_v.paragraphs[0].runs[0].font.size = Pt(10)
        cell_k.width = Inches(1.8)
        cell_v.width = Inches(4.7)

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    def add_heading(text, level=1):
        h = doc.add_heading(level=level)
        h.paragraph_format.space_before = Pt(14)
        h.paragraph_format.space_after = Pt(4)
        r = h.add_run(text)
        r.font.name = "Arial"
        r.font.bold = True
        if level == 1:
            r.font.size = Pt(16)
            r.font.color.rgb = NAVY
        else:
            r.font.size = Pt(13)
            r.font.color.rgb = DARK_GRAY
        return h

    def add_body(text, bold_prefix=None, space_after=6):
        p = doc.add_paragraph()
        p.paragraph_format.space_after = Pt(space_after)
        p.paragraph_format.line_spacing = 1.15
        if bold_prefix:
            rb = p.add_run(bold_prefix)
            rb.font.name = "Arial"
            rb.font.size = Pt(10.5)
            rb.font.bold = True
        r = p.add_run(text)
        r.font.name = "Arial"
        r.font.size = Pt(10.5)
        return p

    # 1. Executive Summary
    add_heading("1. Executive Summary & Objectives")
    add_body("Modern operating systems rely heavily on multi-threading for concurrency and system responsiveness. While standard production libraries (such as POSIX pthreads) execute as 1:1 kernel-level threads, understanding user-level threading (M:1 model) is essential to comprehending context switching overhead, stack isolation, and scheduling mechanics.")
    add_body("The objective of this project is to develop a fully functional, preemptive user-level thread library in C from first principles without utilizing pthread_create(). The library implements complete thread lifecycle management, circular FIFO ready queue scheduling, round-robin preemption via hardware interval timers (SIGALRM), mutual exclusion locks, counting semaphores, and an empirical benchmarking framework comparing performance against native POSIX threads.")

    # 2. Team Division
    add_heading("2. Team Division & Modular Architecture")
    add_body("To maintain professional software engineering standards, the project was partitioned into distinct ownership domains using Git feature branches:")

    team_table = doc.add_table(rows=4, cols=3)
    team_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    headers = ["Member", "Assigned Module", "Owned Source Files & Deliverables"]
    for i, h in enumerate(headers):
        cell = team_table.cell(0, i)
        cell.text = h
        shading = parse_xml(r'<w:shd {} w:fill="1A5276"/>'.format(nsdecls('w')))
        cell._tc.get_or_add_tcPr().append(shading)
        run = cell.paragraphs[0].runs[0]
        run.font.bold = True
        run.font.name = "Arial"
        run.font.color.rgb = RGBColor(255, 255, 255)
        run.font.size = Pt(10)

    rows = [
        ("Person 1", "Thread Lifecycle & State Machine", "src/thread.c, src/queue.c, include/mthread.h, automated test suite (tests/)"),
        ("Person 2", "Scheduling & Timer Preemption", "src/sched.c, src/timer.c, include/sched.h, include/timer.h, demos/preemption_demo.c"),
        ("Person 3", "Synchronization & Concurrency", "src/sync.c, demos/race_condition.c, demos/producer_consumer.c")
    ]
    for row_idx, data in enumerate(rows, start=1):
        for col_idx, text in enumerate(data):
            cell = team_table.cell(row_idx, col_idx)
            cell.text = text
            run = cell.paragraphs[0].runs[0]
            run.font.name = "Arial"
            run.font.size = Pt(9.5)
            if col_idx == 0:
                run.font.bold = True

    doc.add_paragraph().paragraph_format.space_after = Pt(8)

    # 3. Technical Implementation
    add_heading("3. Technical Implementation Details")
    add_heading("3.1 Thread Control Block & Context Switching (Person 1)", level=2)
    add_body("Each user-level thread is represented by a Thread Control Block (TCB) maintaining an identifier, an execution state (READY, RUNNING, BLOCKED, TERMINATED), a dedicated 64 KB user stack, and a POSIX ucontext_t structure. Context switches are performed via swapcontext(), which saves the caller's CPU registers and switches the Stack Pointer (SP) and Program Counter (PC) directly in user space without incurring the overhead of a kernel trap.")

    add_heading("3.2 Preemptive Round-Robin Scheduling (Person 2)", level=2)
    add_body("To prevent CPU-bound tasks from starving cooperating threads, the scheduler operates under a preemptive Round-Robin policy with a default time quantum of 10 ms (10,000 μs). A real-time interval timer is initialized via setitimer(ITIMER_REAL, ...), delivering periodic SIGALRM interrupts. The signal handler invokes sched_tick(), which masks signals via sigprocmask() to prevent re-entrant race conditions, rotates the ready queue, and triggers mt_yield().")

    add_heading("3.3 Synchronization Primitives (Person 3)", level=2)
    add_body("Mutual exclusion is implemented via mt_mutex_t (providing mt_mutex_lock and mt_mutex_unlock), ensuring that only one thread accesses a critical section at any instant. Counting semaphores (mt_sem_t) manage integer token pools (mt_sem_wait, mt_sem_post), coordinating resource access in producer-consumer scenarios.")

    # 4. Empirical Benchmarks
    add_heading("4. Empirical Benchmark: Mini-Thread vs. POSIX Threads")
    add_body("A dedicated benchmarking suite (demos/benchmark_posix.c) was executed to measure the performance differences between user-level threading and POSIX kernel-level threads (pthread). The results are summarized below:")

    bench_table = doc.add_table(rows=4, cols=4)
    bench_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    b_headers = ["Benchmark Dimension", "User-Level Mini-Thread", "POSIX Kernel Thread", "Performance Advantage"]
    for i, h in enumerate(b_headers):
        cell = bench_table.cell(0, i)
        cell.text = h
        shading = parse_xml(r'<w:shd {} w:fill="2C3E50"/>'.format(nsdecls('w')))
        cell._tc.get_or_add_tcPr().append(shading)
        run = cell.paragraphs[0].runs[0]
        run.font.bold = True
        run.font.name = "Arial"
        run.font.color.rgb = RGBColor(255, 255, 255)
        run.font.size = Pt(9.5)

    b_rows = [
        ("Thread Creation & Join (500 iters)", "4.12 μs / thread", "32.49 μs / thread", "Mini-Thread is ~7.9x Faster"),
        ("Mutex Lock + Unlock (100k ops)", "11.23 ns / op", "15.84 ns / op", "Mini-Thread is ~1.4x Faster"),
        ("Context Switch Latency (50k yields)", "902.82 ns (User space)", "System Call Trap Latency", "Zero Kernel Traps")
    ]
    for row_idx, data in enumerate(b_rows, start=1):
        for col_idx, text in enumerate(data):
            cell = bench_table.cell(row_idx, col_idx)
            cell.text = text
            run = cell.paragraphs[0].runs[0]
            run.font.name = "Arial"
            run.font.size = Pt(9.5)
            if col_idx == 3:
                run.font.bold = True
                run.font.color.rgb = RGBColor(39, 174, 96)

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # Insert Chart Image if available
    img_path = "/Users/ivygupta/Downloads/OS PROj/mini-thread-library/benchmark_results.png"
    if os.path.exists(img_path):
        add_heading("4.1 Graphical Performance Comparison", level=2)
        p_img = doc.add_paragraph()
        p_img.alignment = WD_ALIGN_PARAGRAPH.CENTER
        run_img = p_img.add_run()
        run_img.add_picture(img_path, width=Inches(6.2))
        p_cap = doc.add_paragraph()
        p_cap.alignment = WD_ALIGN_PARAGRAPH.CENTER
        r_cap = p_cap.add_run("Figure 1: Empirical Benchmark Comparison — Thread Creation and Mutex Overhead")
        r_cap.font.name = "Arial"
        r_cap.font.size = Pt(9)
        r_cap.font.italic = True

    # 5. Analysis & Findings
    add_heading("5. Architectural Analysis & Trade-Offs")
    add_body("The benchmark data demonstrates fundamental operating system principles:", bold_prefix="Key Finding 1: Creation Speedup. ")
    add_body("User-level threads allocate memory and initialize registers entirely in user space. Conversely, POSIX pthread_create() invokes the Linux clone() system call, creating a kernel task_struct, allocating a separate kernel stack, and updating OS scheduler queues, introducing 7.9x higher creation latency.")
    add_body("While user-level threads provide ultra-low overhead, the M:1 model has trade-offs: all user threads share a single OS process thread, meaning they cannot achieve multi-core hardware parallelism, and any unhandled blocking I/O call halts the entire library process. POSIX threads utilize true multi-core hardware execution.", bold_prefix="Key Finding 2: Trade-Offs. ")

    # 6. Verification
    add_heading("6. Verification & Demonstration Suites")
    add_body("1. Preemption Demo (demos/preemption_demo.c): Three CPU-bound threads executing long busy-loops are automatically preempted upon timer expiration, proving time-slice fairness without voluntary mt_yield() calls.")
    add_body("2. Race Condition Demo (demos/race_condition.c): Demonstrates counter corruption (expected 10, actual 5) due to interleaved execution without locks, and 100% correctness (10/10) when protected by mt_mutex_t.")
    add_body("3. Producer-Consumer Demo (demos/producer_consumer.c): Demonstrates bounded buffer coordination using counting semaphores and mutex protection without buffer underflows or overflows.")
    add_body("4. Automated Unit Test Suite (make test): Executes four automated test harnesses validating circular queue bounds, state transitions, blocking APIs, and edge-case argument validation.")

    output_doc_path = "/Users/ivygupta/Downloads/OS PROj/mini-thread-library/Mini_Thread_Library_Project_Report.docx"
    doc.save(output_doc_path)
    print(f"Report successfully saved to {output_doc_path}")

if __name__ == "__main__":
    create_report()
