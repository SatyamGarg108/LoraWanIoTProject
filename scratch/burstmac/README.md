# Burst-MAC NS-3 Implementation

This directory contains the implementation of Burst-MAC, a protocol for handling bursty traffic in LoRaWAN networks using Virtual Channels and Slot-based scheduling.

## Structure

- **src/**:
  - `burst-mac-app.h/.cc`: End-device application logic (Burst detection, Mode switching, Slot transmission).
  - `burst-scheduler.h/.cc`: Network Server logic (Virtual Channel grouping, Slot assignment).
  - `burst-mac-tag.h/.cc`: Packet tag for control signaling (Burst Requests, Slot Assignments).
- **burstmac-simulation.cc**: Main simulation script.
- **analysis/**: Python scripts for metrics extraction and plotting.

## Compilation

The project is set up as an NS-3 scratch module.

```bash
./ns3 build
```

## Running

Run the simulation using the built executable:

```bash
./build/burstmac-simulation --nNodes=50 --burstPercent=50
```

## Analysis

Redirect output to a log file and analyze:

```bash
./build/burstmac-simulation > simulation.log
python3 scratch/burstmac/analysis/extract_metrics.py simulation.log
```

## Architecture

1. **Burst Detection**: Nodes switch to "Burst Mode" at configured times. They send a "Burst Request" packet tagged with `BurstMacTag`.
2. **Virtual Channels**: The `BurstScheduler` groups nodes based on Spreading Factor (SF) and Frequency (extracted from `LoraTag`).
3. **Scheduling**: The Scheduler assigns unique time slots to nodes within each Virtual Channel to avoid collisions.
4. **Piggybacking**: Slot assignments are logically passed back to nodes (simulated via shared scheduler object for simplicity in this scratch implementation).
5. **Transmission**: Nodes transmit data packets only in their assigned time slots relative to the superframe.
