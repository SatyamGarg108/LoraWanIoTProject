import sys
import re
import matplotlib.pyplot as plt

def parse_logs(logfile):
    sent = 0
    received = 0
    pdr = 0.0
    
    # Regex for final output from LoraPacketTracker
    # Format usually: "123 45" (sent received)
    # But our simulation prints it at the end
    
    with open(logfile, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if "Results" in line:
                continue
            # Try to parse "sent received"
            parts = line.strip().split()
            if len(parts) == 2 and parts[0].isdigit() and parts[1].isdigit():
                sent = int(parts[0])
                received = int(parts[1])
    
    if sent > 0:
        pdr = (received / sent) * 100.0
    
    return sent, received, pdr

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python extract_metrics.py <logfile>")
        sys.exit(1)
        
    s, r, p = parse_logs(sys.argv[1])
    print(f"Sent: {s}, Received: {r}, PDR: {p:.2f}%")
