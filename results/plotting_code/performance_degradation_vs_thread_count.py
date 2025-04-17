import pandas as pd
import matplotlib.pyplot as plt

malloc_df = pd.read_csv("libc_malloc_scalability.csv", delimiter='\t')
custom_df = pd.read_csv("reaper_scalability.csv", delimiter='\t')  # Reaper
hoard_df = pd.read_csv("hoard_scalability.csv", delimiter='\t')

target_iter = 1_000_000
row_idx = malloc_df[malloc_df['Iterations'] == target_iter].index[0]

thread_cols = malloc_df.columns[1:]
thread_counts = [int(col) for col in thread_cols]

malloc_times = malloc_df.loc[row_idx, thread_cols]
reaper_times = custom_df.loc[row_idx, thread_cols]
hoard_times = hoard_df.loc[row_idx, thread_cols]

malloc_base = malloc_times[str(1)]
reaper_base = reaper_times[str(1)]
hoard_base = hoard_times[str(1)]

degradation_malloc = malloc_times / malloc_base
degradation_reaper = reaper_times / reaper_base
degradation_hoard = hoard_times / hoard_base

plt.figure(figsize=(10, 6))
plt.plot(thread_counts, degradation_malloc, marker='o', label='libc malloc', linestyle='-')
plt.plot(thread_counts, degradation_reaper, marker='s', label='Reaper', linestyle='--')
plt.plot(thread_counts, degradation_hoard, marker='^', label='Hoard', linestyle='-.')

plt.axhline(1.0, color='gray', linestyle=':', label='Ideal (No Degradation)')
plt.xlabel('Thread Count')
plt.ylabel('Performance Degradation (Relative to 1 Thread)')
plt.title('Performance Degradation vs Thread Count (1M Iterations)')
plt.xticks(thread_counts)
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()
