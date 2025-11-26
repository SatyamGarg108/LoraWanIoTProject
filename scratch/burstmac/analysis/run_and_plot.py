import subprocess
import matplotlib.pyplot as plt
import re
import numpy as np

def run_simulation(n_nodes, burst_percent):
    cmd = [
        "./ns3", "run", 
        f"burstmac-simulation --nNodes={n_nodes} --burstPercent={burst_percent}"
    ]
    result = subprocess.run(cmd, capture_output=True, text=True, cwd="../../../")
    
    # Parse Output
    # Expected format from LoraPacketTracker::CountMacPacketsGlobally: "Sent Received" (space separated)
    # We need to capture the line after "--- Burst-MAC Results ---"
    output = result.stdout
    lines = output.split('\n')
    sent = 0
    received = 0
    
    capture = False
    for line in lines:
        if "--- Burst-MAC Results ---" in line:
            capture = True
            continue
        if capture:
            parts = line.strip().split()
            if len(parts) >= 2:
                sent = float(parts[0])
                received = float(parts[1])
            break
            
    pdr = 0
    if sent > 0:
        pdr = (received / sent) * 100.0
        
    return pdr

def main():
    nodes_list = [50, 100, 200, 500]
    burst_percents = [20, 50, 80, 100]
    
    results = {}
    
    for bp in burst_percents:
        pdrs = []
        for n in nodes_list:
            print(f"Running: Nodes={n}, Burst={bp}%")
            pdr = run_simulation(n, bp)
            pdrs.append(pdr)
            print(f"  -> PDR: {pdr:.2f}%")
        results[bp] = pdrs

    # Plotting
    plt.figure(figsize=(10, 6))
    for bp, pdrs in results.items():
        plt.plot(nodes_list, pdrs, marker='o', label=f'Burst {bp}%')
        
    plt.xlabel('Number of Nodes')
    plt.ylabel('Packet Delivery Ratio (%)')
    plt.title('Burst-MAC Performance: PDR vs Node Density')
    plt.legend()
    plt.grid(True)
    plt.savefig('burstmac_pdr.png')
    print("Plot saved to burstmac_pdr.png")

if __name__ == "__main__":
    main()
