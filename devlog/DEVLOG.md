**Devlog**

- **Date:** 2025-11-24
- **Summary:** Added Burst-MAC experimental code and a mesh-style stress test. Changes are constrained to `scratch/` and `devlog/` and do NOT modify `src/lorawan` upstream files.

- **Files added:**
- `scratch/burst-mac-mesh-sim.cc`: New simulation that creates `nNodes` end-devices and a single gateway, seeds each node with a per-node packet queue (default 20), runs a burst registration where nodes request a slot and then send in assigned slot windows during burst.
- `devlog/DEVLOG.md`: This file (you are reading it).

- **Purpose of `burst-mac-mesh-sim.cc`:**
- Implements a simplified Burst-MAC flow for mesh-style heavy traffic experiments:
  - Each node keeps a queue of packets and sends rare normal traffic until burst starts.
  - At burst start nodes send a short registration packet (tagged) so the Network Server trace callback assigns them a slot.
  - The server stores slot assignments in a simulator-global map; nodes poll that map and then send queued packets in their assigned slot each superframe.
- The simulation uses a single NetworkServer and a single gateway (can be extended to multiple gateways by changing the `nGateways` parameter).

- **Differences vs upstream `signetlabdei/lorawan` (no upstream code modified):**
- New experiment code in `scratch/` only; no edits to `src/lorawan`.
- Uses existing `NetworkServer` trace source `ReceivedPacket` to hook the server-side slot assignment callback from the scratch program.
- Introduces a `BurstMacTag` (scratch-only) used by this experimental app to identify registration packets and carry a source node id in simulation space.

- **Notes & Limitations:**
- This implementation uses an in-process global map `g_nodeAssignedSlot` to communicate slot assignments to nodes. This is a pragmatic substitute for implementing piggybacked downlink slot assignment (which would require changes across `NetworkServer` -> `Forwarder` -> `Gateway` downlink code paths in `src/lorawan`).
- Node identity for assignments uses the ns-3 `Node::GetId()` value (unique in this simulation), not LoRa device addresses. This avoids collisions seen when using uninitialized LoRa device addresses.
- The server assigns slots in first-come-first-served order per VC (virtual channel). Virtual channel keying is simplified in the mesh sim; a more realistic approach would inspect the `LoraTag` frequency and spreading factor and partition nodes accordingly.
- The app enforces that nodes only transmit in their assigned slots during the burst period; this helps measure the PDR effect of scheduling.

- **Next recommended steps:**
- If you want real piggybacked slot messages, implement downlink tagging/piggyback in `NetworkServer` and Gateways (requires edits in `src/lorawan`).
- Add multiple gateways and more realistic VC differentiation (by reading `LoraTag` from uplinks in the server callback). 
- Tune superframe length / slot duration and experiment with queue sizes to reproduce heavy-load scenarios.

