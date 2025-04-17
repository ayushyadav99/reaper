import pandas as pd
import matplotlib.pyplot as plt

# Load the data
malloc_df = pd.read_csv("libc_malloc_scalability.csv", delimiter='\t')
custom_df = pd.read_csv("reaper_scalability.csv", delimiter='\t')  # Reaper
hoard_df = pd.read_csv("hoard_scalability.csv", delimiter='\t')

# Find the row corresponding to 1 million iterations
target_iter = 5_000_000
row_idx = malloc_df[malloc_df['Iterations'] == target_iter].index[0]

# Extract thread count columns
thread_cols = malloc_df.columns[1:]  # skip 'Iterations'
thread_counts = [int(col) for col in thread_cols]

# Compute speedup over malloc at 1M iterations for each thread count
malloc_row = malloc_df.loc[row_idx, thread_cols]
custom_row = custom_df.loc[row_idx, thread_cols]
hoard_row = hoard_df.loc[row_idx, thread_cols]

speedup_reaper = malloc_row / custom_row
speedup_hoard = malloc_row / hoard_row

# Plotting
plt.figure(figsize=(10, 6))
plt.plot(thread_counts, speedup_reaper, marker='o', label='Reaper', linestyle='-')
plt.plot(thread_counts, speedup_hoard, marker='s', label='Hoard', linestyle='--')
plt.axhline(1.0, color='gray', linestyle=':', label='malloc baseline')

plt.title('Speedup Over libc malloc vs Thread Count (5M Iterations)')
plt.xlabel('Thread Count')
plt.ylabel('Speedup (malloc_time / other_time)')
plt.xticks(thread_counts)
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()
