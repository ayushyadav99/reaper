import pandas as pd
import matplotlib.pyplot as plt

# Load the data
malloc_df = pd.read_csv("libc_malloc_scalability.csv", delimiter='\t')
custom_df = pd.read_csv("reaper_scalability.csv", delimiter='\t')  # Reaper
hoard_df = pd.read_csv("hoard_scalability.csv", delimiter='\t')

# Use 32 threads
col = '32'
iterations = malloc_df['Iterations']
malloc_times = malloc_df[col]
reaper_times = custom_df[col]
hoard_times = hoard_df[col]

# Plot
plt.figure(figsize=(10, 6))
plt.plot(iterations, malloc_times, marker='o', label='libc malloc', linestyle='-')
plt.plot(iterations, reaper_times, marker='s', label='Reaper', linestyle='--')
plt.plot(iterations, hoard_times, marker='^', label='Hoard', linestyle='-.')

plt.xscale('log')
plt.xlabel('Iteration Count (log scale)')
plt.ylabel('Raw Time (lower is better)')
plt.title('Raw Allocation Time vs Iteration Count (32 Threads)')
plt.legend()
plt.grid(True, which='both', linestyle=':')
plt.tight_layout()
plt.show()
