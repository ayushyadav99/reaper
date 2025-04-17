import pandas as pd
import matplotlib.pyplot as plt

threads = [1, 2, 4, 8, 10, 16, 24, 32, 40, 48, 56, 64]

hoard_time = [8.871067, 21.650911, 11.574248, 6.43895, 4.473223, 2.853744,
              2.134468, 1.743896, 1.230473, 1.094107, 1.216615, 1.194077]

system_time = [8.872119, 4.459313, 2.239852, 1.195037, 0.989754, 0.736382,
               0.523982, 0.419555, 0.327571, 0.333948, 0.273564, 0.349753]

reaper_time = [8.878754, 4.448177, 2.243259, 1.139227, 0.957211, 0.650185,
             0.511349, 0.410062, 0.350197, 0.316514, 0.261358, 0.23644]


df = pd.DataFrame({
    'threads': threads,
    'hoard': hoard_time,
    'system': system_time,
    'reaper' : reaper_time
})

df['hoard_speedup'] = df['hoard'].iloc[0] / df['hoard']
df['system_speedup'] = df['system'].iloc[0] / df['system']
df['reaper_speedup'] = df['reaper'].iloc[0] / df['reaper']

plt.figure(figsize=(10, 6))
plt.plot(df['threads'], df['hoard_speedup'], marker='o', label='Hoard Speedup')
plt.plot(df['threads'], df['system_speedup'], marker='s', label='System malloc Speedup')
plt.plot(df['threads'], df['reaper_speedup'], marker='x', label='Reaper Speedup')
plt.plot(df['threads'], df['threads'], linestyle='--', color='gray', label='Ideal Speedup')

plt.title('Speedup vs Number of Threads (Hoard vs System malloc vs Reaper)')
plt.xlabel('Number of Threads')
plt.ylabel('Speedup')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
