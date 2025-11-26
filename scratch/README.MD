# Burst-MAC Implementation (ns-3 / LoRaWAN)

This README documents the Burst-MAC work implemented inside this ns-3 workspace (SignetLab LoRaWAN module). It explains what was added, where files live, how the implementation works, what behaviors it models (compared to the standard MAC), and how to run/use the code and experiments.

## Summary

- Implemented a prototype of "Burst-MAC" on top of the existing LoRaWAN module in this repo. The goal was to model nodes that normally send low-rate traffic but switch to a high-rate burst mode (e.g., after an event), and to coordinate those bursts via a centralized scheduler.
- The implementation includes packet tagging, a centralized scheduler (server-side), a node application that enforces slots and superframes (Class-B-like), helper code to install the app, example simulations, and an analysis script.

## What I changed / added (file map)

- `scratch/burstmac/` (new modular example)
  - `src/burst-mac-tag.h` / `src/burst-mac-tag.cc` — Packet tag used to mark burst registrations and carry a (simple) source id / slot metadata.
  - `src/burst-scheduler.h` / `src/burst-scheduler.cc` — Centralized scheduler installed on the NetworkServer. Groups nodes by Virtual Channel (SF + frequency), assigns slots using a simple hash + linear assignment strategy, and exposes `GetAssignedSlot(nodeId)` used by node apps.
  - `src/burst-node-app.h` / `src/burst-node-app.cc` — Node application that manages Normal <-> Burst mode switching, sends registration packets, polls the scheduler for slot assignments, and enforces slot transmissions using superframes and slots (simple Class-B-like logic). Implements a naive randomized normal traffic generator and precise slot transmissions in burst mode.
  - `src/burst-mac-helper.h` / `src/burst-mac-helper.cc` — Helper to install the `BurstNodeApp` on end devices easily.
  - `examples/burstmac-simulation.cc` — Example scenario that builds a network (end devices, gateways, network server) and demonstrates burst operation. Installs energy models and uses `LoraPacketTracker` at the end to report sent/received counts.
  - `CMakeLists.txt` — Build target for the example inside `scratch/burstmac`.

- `scratch/burst-mac-sim.cc` — An earlier single-file prototype kept in the workspace (contains a compact prototype of the same ideas: tag, VC state, server handler, and application code). Useful as a quick reference.
- `scratch/burst-mac-mesh-sim.cc` — A mesh-style scratch program used to simulate router-style behavior and stress-testing burst scheduling; kept for experiments.
- `analysis/run_and_plot.py` — Python script to automate batch runs and plot PDR results (PDR computed from the built-in `LoraPacketTracker` output). This script helps sweep parameters like node count and burst percent.
- `scratch/How to run.txt` — Step-by-step instructions for building and running the example (added by me).
- `devlog/DEVLOG.md` — Developer notes summarizing the design decisions, differences vs upstream, and debugging notes (created during work).

## Core design & how it works

1. Packet Tagging (node -> server)
   - End devices include a `BurstMacTag` on a registration uplink when they enter burst mode. The tag marks the packet as a burst registration and carries a simple source node identifier. The tag is small and serializable with ns-3 tagging.

2. Server-side Scheduler (centralized)
   - The `BurstScheduler` is instantiated on the NetworkServer node and is hooked to the server trace `ReceivedPacket`. When the server receives a packet with a burst tag, it groups the node into a Virtual Channel (VC) identified by the PHY parameters (spreading factor + frequency) and assigns a slot.
   - Assignment strategy used in this prototype: naive hash (nodeId % groupSize) to choose an ideal slot, but final assignment is guaranteed unique via a linear next-free-slot counter per VC. Assignments are stored in a simulator-side map for simplicity.

3. Node Behavior (class-based enforcement)
   - `BurstNodeApp` runs on end devices. In normal mode it sends occasional low-rate packets. When triggered (simulated by calling `TriggerBurst()`), it sends a registration packet (with the `BurstMacTag`) and begins polling the scheduler for a slot. Once assigned, it aligns to a superframe and transmits only in its assigned slot at a configurable slot duration (burst slot length).
   - This models a Class-B-like periodic superframe where each node has one slot per superframe.

4. Piggybacking vs simulation substitute
   - A full implementation would piggyback slot assignments onto real LoRaWAN downlinks. Doing that correctly requires changes in the LoRaWAN downlink path. To avoid invasive changes here, slot assignments are stored in a global scheduler map and nodes poll it (simulated piggyback). This provides the intended effect (nodes learn slots) without modifying core lorawan module internals.

5. Energy model and metrics
   - The example installs a `BasicEnergySource` and `LoraRadioEnergyModel` on end devices to allow energy measurements in experiments.
   - The example prints `LoraPacketTracker::CountMacPacketsGlobally` output (sent / received counts). The `analysis/run_and_plot.py` script parses that to compute PDR and make plots.

## How this demonstrates the effect of Burst-MAC vs standard MAC

- Standard MAC (baseline): devices send low-rate traffic with no coordination. Under bursts, many nodes may transmit simultaneously causing collisions at the gateway (especially in LoRa where airtime and capture depend on SF/frequency). PDR typically degrades when many nodes transmit opportunistically.
- Burst-MAC (this prototype): nodes register and receive assigned slots per VC. Slot enforcement confines high-rate transmissions to non-overlapping time slots (within VC), which reduces collisions in time for similarly-configured nodes and can increase PDR during bursts. The scheduler groups nodes by PHY parameters (SF + frequency) so VCs approximate separate interference domains.
- Observable effects you can measure with these experiments:
  - Packet Delivery Ratio (PDR): compare sent vs received counts for baseline vs burst-coordinated runs.
  - Energy consumption: burst transmissions concentrated in slots can change TX/RX duty cycles — the energy model records source remaining energy.
  - Throughput & fairness: slot assignment strategy affects how many packets each node can send per superframe; the simple linear assignment causes a first-come-first-served fairness.

Note: The current scheduler is intentionally simple. More advanced strategies (randomized slot selection, slot reuse, dynamic slot reclamation, downlink piggybacking) can be implemented and plugged into the same node app / example harness.

## Feature list (what the code can do)

- Burst registration tagging — nodes announce intent to burst using a `BurstMacTag`.
- Centralized VC grouping and slot assignment (`BurstScheduler`).
- Node-side application (`BurstNodeApp`) that:
  - switches modes (Normal / Burst),
  - registers with the scheduler,
  - polls/waits for slot assignments,
  - aligns to superframes and transmits only in assigned slots,
  - falls back to normal random periodic transmissions when not in burst mode.
- Helper (`BurstMacHelper`) to install the burst app on nodes.
- Example `burstmac-simulation` that builds a simple network, installs energy models, and prints final MAC-level metrics.
- Analysis script (`analysis/run_and_plot.py`) to run sweeps and compute/plot PDR results.
- Multiple prototypes / references remain in `scratch/` for quick experimentation: `burst-mac-sim.cc` (single-file), `burst-mac-mesh-sim.cc` (mesh-style experiment).

## How to run (quick)

See `scratch/How to run.txt` — it contains step-by-step build & run instructions. In short:

1. Build the project via the top-level wrapper in project root:
   - `./ns3` (or use CMake: `mkdir -p cmake-cache && /usr/bin/cmake --build cmake-cache -j 5`)
2. Export library path:
   - `export LD_LIBRARY_PATH=/home/satyamwsl/IoT/ns-3-dev/build/lib:$LD_LIBRARY_PATH`
3. Run the example (smoke test):
   - `/home/satyamwsl/IoT/ns-3-dev/build/burstmac-simulation --nNodes=20 --nGateways=1 --burstPercent=50`

Command-line options the example accepts (examples in `How to run.txt` too):
- `--nNodes`, `--nGateways`, `--burstPercent`, `--nChannels`.

## Limitations & known issues

- Piggybacking is simulated by polling: the scheduler stores assignments in a simulator-side map nodes poll. This is a pragmatic shortcut — not a full downlink piggyback implementation.
- Scheduler is single-server and simple. It does not reclaim slots, nor handle node departures elegantly. Assignments are persistent for the simulation runtime.
- VC grouping uses basic PHY tags available in packet traces; in the real LoRaWAN world VCs can be more complex (time/frequency/SF combinations, gateway diversity). The prototype groups by SF and frequency only.
- The assignment algorithm is naive (hash + next-free-slot). It demonstrates collision resolution but is not optimized for utilization.
- The code is intentionally written with simpler / novice-style names in parts (per request) — variable names and comments are straightforward but not production polished.

## Where to go next (extensions you might want)

- Implement true downlink piggybacking by extending the lorawan downlink path to carry slot assignment TLVs.
- Add dynamic slot reclamation and slot lifetimes to allow slot reuse.
- Improve scheduling algorithms (randomized, fairness-aware, priority-based).
- Add statistical logging hooks (per-slot utilization, collisions, per-node latency) and export CSV for analysis.
- Add a test harness that compares baseline vs Burst-MAC automatically and plots PDR/energy tradeoffs for parameter sweeps (the `analysis/` script can be expanded for that).

## Contact / Notes

All changes are in the `scratch/` folder so they do not modify core upstream code. If you want the scheduler implemented inside `src/lorawan` (more realistic integration), that will require deeper edits to downlink code, interfaces, and tests.

If you'd like, I can:
- finish the novice-style renaming consistently across all files (I already applied most renames),
- run a smoke test here and paste the output,
- or implement a simple slot-reclaim policy in the `BurstScheduler`.

---

Files referenced above are in the project workspace under `scratch/burstmac/` and `scratch/` — open them if you want code pointers to specific functions/classes.
