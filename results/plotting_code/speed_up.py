import pandas as pd
import matplotlib.pyplot as plt

malloc_df = pd.read_csv("libc_malloc_scalability.csv", delimiter='\t')
hoard_df = pd.read_csv("hoard_scalability.csv", delimiter='\t')
custom_df = pd.read_csv("reaper_scalability.csv", delimiter='\t')

target_iter = 1_000_000
malloc_row = malloc_df[malloc_df['Iterations'] == target_iter].iloc[0]
hoard_row = hoard_df[hoard_df['Iterations'] == target_iter].iloc[0]
custom_row = custom_df[custom_df['Iterations'] == target_iter].iloc[0]

thread_cols = malloc_df.columns[1:]
thread_counts = thread_cols.astype(int)

malloc_base = malloc_row['1']
hoard_base = hoard_row['1']
custom_base = custom_row['1']

malloc_speedup = malloc_base / malloc_row[thread_cols].astype(float)
hoard_speedup = hoard_base / hoard_row[thread_cols].astype(float)
custom_speedup = custom_base / custom_row[thread_cols].astype(float)

plt.figure(figsize=(10, 6))
plt.plot(thread_counts, malloc_speedup, marker='o', linestyle='-', label='libc malloc')
plt.plot(thread_counts, hoard_speedup, marker='o', linestyle='--', label='Hoard')
plt.plot(thread_counts, custom_speedup, marker='o', linestyle='-.', label='Reaper')

plt.title('Allocator Scalability: Speedup vs Thread Count (1M Iterations)')
plt.xlabel('Thread Count')
plt.ylabel('Speedup (1 Thread Time / N Thread Time)')
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()
