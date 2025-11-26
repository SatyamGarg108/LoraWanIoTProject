// File: burst-mac-tag.h
// Purpose: Small packet tag used to mark burst packets and carry a simple
// control payload (sender id and assigned slot). The tag is used by both
// node-side apps and the scheduler/server-side logic.

#ifndef BURST_MAC_TAG_H
#define BURST_MAC_TAG_H

#include "ns3/tag.h"
#include "ns3/type-id.h"

namespace ns3 {

/**
 * @brief Tag used to carry Burst-MAC control information.
 *
 * Fields:
 * - IsBurst: True if the packet is part of a burst.
 * - SourceNodeId: The ID of the sender (for server identification).
 * - AssignedSlot: The slot assigned by the server (simulated downlink).
 */
class BurstMacTag : public Tag
{
public:
  // simple novice-style API
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const override;
  uint32_t GetSerializedSize (void) const override;
  void Serialize (TagBuffer i) const override;
  void Deserialize (TagBuffer i) override;
  void Print (std::ostream &os) const override;

  // flag helpers
  void set_burst (bool b);
  bool is_burst (void) const;

  // source and slot helpers
  void set_src (uint32_t id);
  uint32_t src (void) const;

  void set_slot (uint32_t s);
  uint32_t slot (void) const;

private:
  // very simple member names
  bool is_burst_flag = false;
  uint32_t src_node = 0;
  uint32_t assigned_slot = 0;
};

} // namespace ns3

#endif /* BURST_MAC_TAG_H */
