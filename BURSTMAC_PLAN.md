# Burst-MAC Implementation Plan

## Phase 1: Baseline Setup & Verification
**Goal:** Ensure the standard LoRaWAN simulation works and establish a baseline for performance comparison.
- [ ] **Verify Environment:** Check `ns-3-dev` compilation with the LoRaWAN module.
- [ ] **Run Baseline:** Execute `scratch/burstmac/examples/burstmac-simulation.cc` with default ALOHA settings.
- [ ] **Record Metrics:** Measure Packet Reception Ratio (PRR) to confirm the "Problem" (collisions under load).

## Phase 2: Burst Detection (Node & Gateway)
**Goal:** Enable nodes and gateway to detect high traffic conditions.
- [ ] **Modify `EndDeviceLorawanMac`:**
    - Add `m_packetRate` tracking (e.g., packets per minute).
    - Add `m_burstThreshold` attribute.
    - Implement logic: If `rate > threshold`, set `m_isBurstMode = true`.
- [ ] **Modify `LorawanMacHeader`:**
    - Use a reserved bit (RFU) in the MHDR (MAC Header) to represent the `BurstBit`.
    - Update `Serialize` and `Deserialize` methods to handle this bit.
- [ ] **Modify `GatewayLorawanMac` / `NetworkServer`:**
    - In `Receive()`, check the `BurstBit`.
    - Implement a collision counter per channel/SF to detect network-wide bursts.

## Phase 3: Grouping into Virtual Channels (VCs)
**Goal:** Logically group nodes to manage interference.
- [ ] **Define Virtual Channel:** A VC is defined by the pair `(Frequency, SpreadingFactor)`.
- [ ] **Node Logic:** Nodes already know their Frequency and SF. No major change needed, just awareness that they belong to this group.
- [ ] **Gateway Logic:** The Network Server must maintain a list of nodes per VC to manage the schedule.

## Phase 4: Hash-Based Scheduling (TDMA)
**Goal:** Assign time slots to nodes to avoid collisions within a VC.
- [ ] **Implement Hashing:**
    - Function: `SlotID = NodeID % GroupSize`.
    - `GroupSize` needs to be known or estimated (can be broadcasted by Gateway or static for simulation).
- [ ] **Implement Slot Timing:**
    - Calculate `SlotDuration` based on SF (e.g., SF7=L, SF8=2L).
    - Implement a "Superframe" structure.
- [ ] **Modify Transmission Logic (`EndDeviceLorawanMac`):**
    - If `m_isBurstMode` is true:
        - Do NOT transmit immediately.
        - Calculate `Delay = TimeToNextSlot`.
        - Schedule `Send()` event after `Delay`.

## Phase 5: Collision Resolution
**Goal:** Handle cases where `NodeID % GroupSize` results in the same slot.
- [ ] **Gateway Detection:**
    - The Network Server checks if two nodes in the same VC are trying to use the same slot.
- [ ] **Reassignment Logic:**
    - Find a free slot in the Superframe.
    - Create a mechanism to send this "New Slot" to the node.
    - **Option:** Use a custom MAC Command or piggyback on the ACK.
- [ ] **Node Handling:**
    - Process the reassignment command and update `SlotID`.

## Phase 6: Mode Switching & Class B
**Goal:** Return to normal operation when traffic subsides.
- [ ] **Beaconing:** Implement a simple Beacon from Gateway to synchronize time (essential for TDMA).
- [ ] **Switch Back:**
    - If `packetRate < threshold`, set `m_isBurstMode = false`.
    - Stop using TDMA, revert to ALOHA.

## Phase 7: Simulation & Evaluation
**Goal:** Generate results.
- [ ] **Scenarios:**
    - Vary Burst Nodes (20-100%).
    - Vary Network Size (200-1000).
- [ ] **Metrics:**
    - PRR (Packet Reception Ratio).
    - Latency.
    - Energy Consumption.
