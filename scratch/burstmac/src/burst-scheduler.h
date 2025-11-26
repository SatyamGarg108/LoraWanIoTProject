// File: burst-scheduler.h
// Purpose: Very small centralized scheduler used in the Burst-MAC prototype.
// The scheduler groups nodes by a simple VirtualChannel key (data rate + freq)
// and assigns incremental slots to nodes that send a registration (uplink)
// packet marked as a burst. This is intentionally lightweight for example
// and teaching purposes.

#ifndef BURST_SCHEDULER_H
#define BURST_SCHEDULER_H

#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/lora-tag.h"
#include <map>
#include <vector>

namespace ns3 {

/**
 * @brief Virtual Channel identifier (SF + Frequency).
 */
struct VirtualChannel {
  uint8_t sf;
  double freq;

  bool operator< (const VirtualChannel& other) const {
    if (sf != other.sf) return sf < other.sf;
    return freq < other.freq;
  }
};

/**
 * @brief State of a Virtual Channel.
 *
 * Holds the list of observed node ids that declared a burst and a mapping
 * of assigned slots. `nextFreeSlot` is used to give new arriving nodes a
 * unique slot index (simple incremental allocation).
 */
struct VCState {
  std::vector<uint32_t> nodeIds;
  std::map<uint32_t, uint32_t> slotAssignments;
  uint32_t nextFreeSlot = 0;
};

/**
 * @brief Centralized Scheduler for Burst-MAC.
 *
 * Handles:
 * - Virtual Channel Grouping
 * - Hash-based Slot Scheduling (simple incremental assignment here)
 * - Exposes a lookup API so nodes can query their assigned slot.
 */
class BurstScheduler : public Object
{
public:
  // written to look a bit naive and simple
  static TypeId GetTypeId (void);
  BurstScheduler ();
  virtual ~BurstScheduler ();

  // process an uplink packet and maybe give a slot
  void OnUplink (Ptr<const Packet> packet);

  // ask the scheduler which slot a node has (or UINT32_MAX)
  uint32_t GetAssignedSlot (uint32_t nodeId);

private:
  std::map<VirtualChannel, VCState> vc_map; // virtual-channel -> state
  std::map<uint32_t, uint32_t> node_slot_map; // node -> slot
};

} // namespace ns3

#endif /* BURST_SCHEDULER_H */
