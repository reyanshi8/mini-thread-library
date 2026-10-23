#!/bin/bash
# ========================================================
# Mini-Thread vs POSIX Thread Benchmark Plotter
# Generates high-quality charts for project report & PPT
# ========================================================

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

# Set style
plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
fig, axes = plt.subplots(1, 2, figsize=(14, 5))

# ----------------------------------------------------
# Chart 1: Thread Creation & Termination Latency (us)
# ----------------------------------------------------
categories = ['Thread Creation & Join\n(Average Latency per Thread)']
mini_thread_creation = [4.12]
posix_thread_creation = [32.49]

x = np.arange(len(categories))
width = 0.35

rects1 = axes[0].bar(x - width/2, mini_thread_creation, width, label='User-Level Mini-Thread', color='#2ecc71', edgecolor='black')
rects2 = axes[0].bar(x + width/2, posix_thread_creation, width, label='POSIX Kernel Thread (pthread)', color='#e74c3c', edgecolor='black')

axes[0].set_ylabel('Time in Microseconds (μs) [Lower is Better]', fontsize=11, fontweight='bold')
axes[0].set_title('Thread Creation & Termination Overhead\n(User-Level vs Kernel-Level)', fontsize=13, fontweight='bold')
axes[0].set_xticks(x)
axes[0].set_xticklabels(categories, fontsize=11)
axes[0].legend(fontsize=10)
axes[0].grid(True, linestyle='--', alpha=0.6)

# Add values above bars
for rect in rects1:
    height = rect.get_height()
    axes[0].annotate(f'{height:.2f} μs\n(~8x Faster)',
                     xy=(rect.get_x() + rect.get_width() / 2, height),
                     xytext=(0, 3), textcoords="offset points",
                     ha='center', va='bottom', fontweight='bold', color='#1b7a43')

for rect in rects2:
    height = rect.get_height()
    axes[0].annotate(f'{height:.2f} μs',
                     xy=(rect.get_x() + rect.get_width() / 2, height),
                     xytext=(0, 3), textcoords="offset points",
                     ha='center', va='bottom', fontweight='bold', color='#a93226')

# ----------------------------------------------------
# Chart 2: Mutex Lock/Unlock Operation Latency (ns)
# ----------------------------------------------------
categories_mutex = ['Mutex Lock + Unlock\n(Per Operation Latency)']
mini_thread_mutex = [11.23]
posix_mutex = [15.84]

x2 = np.arange(len(categories_mutex))
rects3 = axes[1].bar(x2 - width/2, mini_thread_mutex, width, label='Mini-Thread Mutex', color='#3498db', edgecolor='black')
rects4 = axes[1].bar(x2 + width/2, posix_mutex, width, label='POSIX pthread_mutex', color='#9b59b6', edgecolor='black')

axes[1].set_ylabel('Time in Nanoseconds (ns) [Lower is Better]', fontsize=11, fontweight='bold')
axes[1].set_title('Synchronization Overhead\n(Lock Acquisition & Release)', fontsize=13, fontweight='bold')
axes[1].set_xticks(x2)
axes[1].set_xticklabels(categories_mutex, fontsize=11)
axes[1].legend(fontsize=10)
axes[1].grid(True, linestyle='--', alpha=0.6)

for rect in rects3:
    height = rect.get_height()
    axes[1].annotate(f'{height:.2f} ns',
                     xy=(rect.get_x() + rect.get_width() / 2, height),
                     xytext=(0, 3), textcoords="offset points",
                     ha='center', va='bottom', fontweight='bold', color='#1f618d')

for rect in rects4:
    height = rect.get_height()
    axes[1].annotate(f'{height:.2f} ns',
                     xy=(rect.get_x() + rect.get_width() / 2, height),
                     xytext=(0, 3), textcoords="offset points",
                     ha='center', va='bottom', fontweight='bold', color='#6c3483')

plt.tight_layout()
output_path = '/Users/ivygupta/Downloads/OS PROj/mini-thread-library/benchmark_results.png'
plt.savefig(output_path, dpi=300)
print(f"Chart saved to {output_path}")
