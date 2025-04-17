import pandas as pd
import matplotlib.pyplot as plt

# Number of threads
threads = [1, 2, 4, 8, 10, 16, 24, 32, 40, 48, 56, 64]


hoard_time = [6.045128,
17.621186,
14.944945,
12.342679,
6.274449,
4.855224,
3.52855,
2.556959,
2.15,
2.259759,
2.00036,
1.578709]

system_time=[6.137851,
17.881593,
8.373469,
4.197094,
3.48319,
2.153418,
1.628823,
1.330842,
1.058413,
0.917474,
0.673732,
0.714545]

reaper_time = [6.197231,
3.065182,
1.580478,
0.790218,
0.633016,
0.433889,
0.339643,
0.265679,
0.220513,
0.187879,
0.178476,
0.234025]


# Create DataFrame
df = pd.DataFrame({
    'threads': threads,
    'hoard': hoard_time,
    'system': system_time,
    'reaper' : reaper_time
})

# Compute speedups
df['hoard_speedup'] = df['hoard'].iloc[0] / df['hoard']
df['system_speedup'] = df['system'].iloc[0] / df['system']
df['reaper_speedup'] = df['reaper'].iloc[0] / df['reaper']

# Plotting
plt.figure(figsize=(10, 6))
plt.plot(df['threads'], df['hoard_speedup'], marker='o', label='Hoard Speedup')
plt.plot(df['threads'], df['system_speedup'], marker='s', label='System malloc Speedup')
plt.plot(df['threads'], df['reaper_speedup'], marker='x', label='Reaper Speedup')
plt.plot(df['threads'], df['threads'], linestyle='--', color='gray', label='Ideal Speedup')

plt.title('Cache Scratch Speedup vs Number of Threads (Hoard vs System malloc vs Reaper)')
plt.xlabel('Number of Threads')
plt.ylabel('Speedup')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()

