import matplotlib.pyplot as plt
import sys

# Example data plotting script
# Usage: python plot_results.py

def plot_pdr_vs_nodes():
    nodes = [20, 50, 100, 200]
    pdr = [95, 85, 60, 40] # Placeholder data
    
    plt.figure()
    plt.plot(nodes, pdr, marker='o')
    plt.title('PDR vs Number of Nodes')
    plt.xlabel('Number of Nodes')
    plt.ylabel('PDR (%)')
    plt.grid(True)
    plt.savefig('pdr_vs_nodes.png')
    print("Generated pdr_vs_nodes.png")

if __name__ == "__main__":
    plot_pdr_vs_nodes()
