#ifndef BURST_SCHEDULER_H
#define BURST_SCHEDULER_H

#include <map>
#include <vector>
#include <stdint.h>
#include "ns3/object.h"
#include "ns3/packet.h"

namespace ns3 {

// Virtual Channel Key
struct VirtualChannelKey {
  uint8_t sf;
  double freq;
  bool operator<(const VirtualChannelKey& other) const {
    if (sf != other.sf) return sf < other.sf;
    return freq < other.freq;
  }
};

// State for a single VC
struct VCState {
  std::vector<uint32_t> nodeIds;
  std::map<uint32_t, uint32_t> assignments; // NodeId -> Slot
  uint32_t nextFreeSlot = 0;
};

/**
 * @brief Centralized scheduler for Burst-MAC.
 * Assigns slots to nodes based on Virtual Channels (SF/Freq).
 */
class BurstScheduler : public Object
{
public:
  static TypeId GetTypeId (void);
  BurstScheduler ();
  virtual ~BurstScheduler ();

  // Called when server receives a packet
  void OnReceivedPacket (Ptr<const Packet> packet);

  // Get assigned slot for a node (simulated piggyback lookup)
  // Returns UINT32_MAX if no assignment
  uint32_t GetAssignedSlot (uint32_t nodeId);

private:
  std::map<VirtualChannelKey, VCState> m_vcStates;
  std::map<uint32_t, uint32_t> m_globalAssignments; // Fast lookup
};

} // namespace ns3

#endif /* BURST_SCHEDULER_H */
