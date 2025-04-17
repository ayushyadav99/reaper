import pandas as pd
import matplotlib.pyplot as plt

# Load your CSV (export the table first)
df = pd.read_csv('reaper_scalability.csv', delimiter='\t')  # Replace with actual file path

thread_counts = df.columns[1:].astype(int)

speedup_df = df.copy()
for col in thread_counts:
    speedup_df[str(col)] = df[str(1)] / df[str(col)]

plt.figure(figsize=(10, 6))
for idx, row in speedup_df.iterrows():
    iterations = row['Iterations']
    speedups = row[1:].values.astype(float)  # skip 'Iterations' column
    plt.plot(thread_counts, speedups, marker='o', label=f"{iterations} iters")

plt.title('Speedup vs Thread Count')
plt.xlabel('Thread Count')
plt.ylabel('Speedup (Time@1Thread / Time@NThreads)')
plt.legend(title='Iterations')
plt.grid(True)
plt.tight_layout()
plt.show()
