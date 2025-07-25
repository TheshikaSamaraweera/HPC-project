import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Load CSV
df = pd.read_csv("results.csv")

# Plot average time
avg_df = df.groupby("method").first().reset_index()

plt.figure(figsize=(10, 6))
sns.barplot(data=avg_df, x="method", y="avg_time", palette="viridis")
plt.title("Average Time Per Prompt by Method")
plt.ylabel("Average Time (s)")
plt.xlabel("Method")
plt.tight_layout()
plt.savefig("avg_times.png")
plt.show()

# Optional: plot per-prompt
plt.figure(figsize=(12, 6))
sns.lineplot(data=df, x="prompt_index", y="time", hue="method", marker="o")
plt.title("Time per Prompt (All Methods)")
plt.ylabel("Time (s)")
plt.xlabel("Prompt Index")
plt.tight_layout()
plt.savefig("per_prompt.png")
plt.show()
