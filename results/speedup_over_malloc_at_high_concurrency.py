import pandas as pd
import matplotlib.pyplot as plt

# Load data
malloc_df = pd.read_csv("libc_malloc_scalability.csv", delimiter='\t')
custom_df = pd.read_csv("reaper_scalability.csv", delimiter='\t')  # Reaper
hoard_df = pd.read_csv("hoard_scalability.csv", delimiter='\t')

# Focus on 32 threads
col = '16'
iterations = malloc_df['Iterations']

# Compute speedups
speedup_reaper = malloc_df[col] / custom_df[col]
speedup_hoard = malloc_df[col] / hoard_df[col]

# Plot
plt.figure(figsize=(10, 6))
plt.plot(iterations, speedup_reaper, marker='o', label='Reaper', linestyle='-')
plt.plot(iterations, speedup_hoard, marker='s', label='Hoard', linestyle='--')

plt.axhline(1.0, color='gray', linestyle=':', label='malloc baseline')
plt.title('Speedup Over libc malloc vs Iterations (16 Threads)')
plt.xlabel('Iteration Count')
plt.ylabel('Speedup (malloc_time / other_time)')
plt.xscale('log')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
