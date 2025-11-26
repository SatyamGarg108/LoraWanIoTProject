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
 * - Hash-based Slot Scheduling
 * - Collision Resolution
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
  std::map<VirtualChannel, VCState> vc_map; // not fancy name
  std::map<uint32_t, uint32_t> node_slot_map; // node -> slot
};

} // namespace ns3

#endif /* BURST_SCHEDULER_H */
