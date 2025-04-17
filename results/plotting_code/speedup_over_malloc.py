import pandas as pd
import matplotlib.pyplot as plt

# Load all three tables (tab-separated)
malloc_df = pd.read_csv("libc_malloc_scalability.csv", delimiter='\t')
hoard_df = pd.read_csv("hoard_scalability.csv", delimiter='\t')
custom_df = pd.read_csv("reaper_scalability.csv", delimiter='\t')

# Filter only the row with 1 million iterations
target_iter = 1_000_000
malloc_row = malloc_df[malloc_df['Iterations'] == target_iter].iloc[0]
hoard_row = hoard_df[hoard_df['Iterations'] == target_iter].iloc[0]
custom_row = custom_df[custom_df['Iterations'] == target_iter].iloc[0]

# Extract thread counts (all columns except 'Iterations')
thread_cols = malloc_df.columns[1:]
thread_counts = thread_cols.astype(int)

# Compute speedup over malloc
speedup_hoard = malloc_row[thread_cols].astype(float) / hoard_row[thread_cols].astype(float)
speedup_custom = malloc_row[thread_cols].astype(float) / custom_row[thread_cols].astype(float)

# Plot
plt.figure(figsize=(10, 6))
plt.plot(thread_counts, speedup_hoard, marker='o', linestyle='--', label='Hoard vs malloc')
plt.plot(thread_counts, speedup_custom, marker='o', linestyle='-', label='Reaper vs malloc')

plt.axhline(1.0, color='black', linestyle=':', label='malloc baseline')
plt.title('Speedup Over libc malloc vs Thread Count (1M Iterations)')
plt.xlabel('Thread Count')
plt.ylabel('Speedup (malloc_time / allocator_time)')
plt.xticks(thread_counts)
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()
