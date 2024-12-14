import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV file
file_path = "gantt_chart.csv"  # Ensure the file path is correct
data = pd.read_csv(file_path)

# Prepare unique processes excluding 'Idle'
unique_processes = [p for p in data['Process'].unique() if p != 'Idle']
unique_processes.append('Idle')  # Add 'Idle' separately at the end

# Assign y positions to processes
y_positions = {process: i for i, process in enumerate(unique_processes)}

# Initialize the Gantt chart plot
fig, ax = plt.subplots(figsize=(12, 6))

# Plot each process
for _, row in data.iterrows():
    process = row['Process']
    ax.broken_barh([(row['Time'], 1)], (y_positions[process], 0.8), 
                   facecolors='grey' if process == 'Idle' else f'C{y_positions[process] % 10}')

# Format the chart
ax.set_yticks(list(y_positions.values()))
ax.set_yticklabels(list(y_positions.keys()))
ax.set_xlabel("Time")
ax.set_ylabel("Processes")
ax.set_title("Gantt Chart for Round Robin Scheduling")

# Add vertical grid lines for every second
ax.set_xticks(range(data['Time'].min(), data['Time'].max() + 1))  # Tick for every second
ax.grid(axis='x', linestyle='--', linewidth=0.5, alpha=0.7)  # Vertical grid lines for x-axis

# Add horizontal grid lines for processes
ax.grid(axis='y', linestyle='--', linewidth=0.5, alpha=0.7)

plt.tight_layout()

# Show or save the chart
plt.show()
