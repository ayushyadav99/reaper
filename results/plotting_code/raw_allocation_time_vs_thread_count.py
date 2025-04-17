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

plt.figure(figsize=(10, 6))
plt.plot(thread_counts, malloc_times, marker='o', label='libc malloc', linestyle='-')
plt.plot(thread_counts, reaper_times, marker='s', label='Reaper', linestyle='--')
plt.plot(thread_counts, hoard_times, marker='^', label='Hoard', linestyle='-.')

plt.xlabel('Thread Count')
plt.ylabel('Raw Time (lower is better)')
plt.title('Raw Allocation Time vs Thread Count (1M Iterations)')
plt.xticks(thread_counts)
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()
