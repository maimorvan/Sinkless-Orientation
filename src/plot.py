import matplotlib.pyplot as plt
import csv

nodes = []
avg_rounds = []

with open('data/savesave.csv', 'r') as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        nodes.append(int(row['nodes']))
        avg_rounds.append(float(row['avg_rounds']))

plt.figure(figsize=(8, 5))
plt.plot(nodes, avg_rounds, marker='o')
plt.xlabel('Number of nodes')
plt.ylabel('Number of rounds')
plt.title('Sinkless Orientation : number of round depending on the size of the graph')
plt.grid(True)
plt.tight_layout()
plt.show()